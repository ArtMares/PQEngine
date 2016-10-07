/****************************************************************************
**
** Copyright (C) 2015 WxMaper (http://wxmaper.ru)
**
** This file is part of the PHPQt5.
**
** BEGIN LICENSE: MPL 2.0
**
** This Source Code Form is subject to the terms of the Mozilla Public
** License, v. 2.0. If a copy of the MPL was not distributed with this
** file, You can obtain one at http://mozilla.org/MPL/2.0/.
**
** END LICENSE
**
****************************************************************************/

#ifndef PHPQT5_H
#define PHPQT5_H

#include "phpqt5objectfactory.h"
#include "plastiqthreadcreator.h"

#include "pqengine.h"

ZEND_BEGIN_ARG_INFO_EX(phpqt5__call, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, args)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(phpqt5__set, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(phpqt5__get, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

#define ZHANDLE(zobject) (zobject)->handle
#define PQZHANDLE_PROP_NAME "__pq_zhandle_"

#define PQTHREAD __pq_th
#define FETCH_PQTHREAD() QThread *PQTHREAD = QThread::currentThread();
#define PQZHANDLE "__pq_zhandle_"

struct ConnectionData {
    PQObjectWrapper *sender;
    QByteArray signal;
    PQObjectWrapper *receiver;
    QByteArray slot;
};

typedef QHash<QByteArray, ConnectionData> ConnectionHash;

struct PQObjectWrapper {
    PlastiQObject *object;
    bool isExtra = false;       // true if object created from return value
    bool isValid = false;       // false if object not created or has destroyed

    zend_object *zoptr;         // closure object
    bool isClosure = false;     // true if this is a Closure object, then zoptr is a pointer to Closure object

    quint64 enumVal = 0;
    bool isEnum = false;

    QHash<QByteArray,ConnectionHash*> *connections = Q_NULLPTR;
    QHash<QByteArray,zval> *userProperties = Q_NULLPTR;

    void *ctx;
    QThread *thread;

    zend_object zo;             // valid object if this is not Closure object
};

struct PQEnumWrapper {
    qint64 enum_val = 0;
    zend_object zo;
};

struct qrc_stream_data {
    QDataStream *qrc_file;
    php_stream *stream;
};

typedef struct _pq_tmp_call_info {
    PlastiQObject *po;
    zval *zv;
    bool haveParent;
} pq_tmp_call_info;

#include "plastiqobject.h"

static inline PQObjectWrapper *fetch_pqobject(zend_object *zobject) {
#ifdef PQDEBUG
    PQDBG_LVL_START(__FUNCTION__);
#endif

    PQObjectWrapper *pqobject = (PQObjectWrapper *) ((char*)(zobject) - XtOffsetOf(PQObjectWrapper, zo));

#ifdef PQDEBUG
    if(pqobject->object != Q_NULLPTR) {
        PQDBGLPUP(QString("fetch: %1:%2 - %4:%5")
                  .arg(zobject->ce->name->val)
                  .arg(zobject->handle)
                  .arg(reinterpret_cast<quint64>(zobject))
                  .arg(reinterpret_cast<quint64>(pqobject->object->plastiq_data())));
    }
    else {
        PQDBGLPUP(QString("fetch: %1:%2:%3 - Q_NULLPTR:0")
                  .arg(zobject->ce->name->val)
                  .arg(zobject->handle)
                  .arg(reinterpret_cast<quint64>(zobject)));
    }
#endif

    PQDBG_LVL_DONE();
    return pqobject;
}

static inline zval fetch_zobject(QObject *qobject)
{
#ifdef PQDEBUG
    PQDBG_LVL_START(__FUNCTION__);
#endif

    zval zv;
    _zend_object *zobject;

    if(qobject) {
        int indexOfMethod = qobject->metaObject()->indexOfMethod(QByteArray("__pq_getZObject()"));

        if(indexOfMethod >= 0
                && qobject->metaObject()->invokeMethod(qobject,
                                                       "__pq_getZObject",
                                                       Q_RETURN_ARG(_zend_object*, zobject))) {

            PQDBGLPUP(QString("fetch zobject: %1").arg(reinterpret_cast<quint64>(zobject)));

            if(zobject == Q_NULLPTR) {
                ZVAL_UNDEF(&zv);
                PQDBGLPUP(QString("fetch: %1:%2:%3 - %4:%5:%6(%7)")
                          .arg("NULL")
                          .arg(-1)
                          .arg(reinterpret_cast<quint64>(zobject))
                          .arg(qobject->metaObject()->className())
                          .arg(qobject->property(PQZHANDLE).toInt())
                          .arg(reinterpret_cast<quint64>(qobject))
                          .arg(qobject->objectName()));
            }
            else {
                ZVAL_OBJ(&zv, zobject);
                PQDBGLPUP(QString("fetch: %1:%2:%3 - %4:%5:%6(%7)")
                          .arg(zobject->ce->name->val)
                          .arg(zobject->handle)
                          .arg(reinterpret_cast<quint64>(zobject))
                          .arg(qobject->metaObject()->className())
                          .arg(qobject->property(PQZHANDLE).toInt())
                          .arg(reinterpret_cast<quint64>(qobject))
                          .arg(qobject->objectName()));
            }
        }
        else {
            ZVAL_NULL(&zv);
            PQDBGLPUP(QString("fetch: %1:%2:%3 - %4:%5:%6(%7)")
                      .arg("NULL")
                      .arg(0)
                      .arg(0)
                      .arg(qobject->metaObject()->className())
                      .arg(qobject->property(PQZHANDLE).toInt())
                      .arg(reinterpret_cast<quint64>(qobject))
                      .arg(qobject->objectName()));
        }
    }
    else {
        ZVAL_NULL(&zv);
    }

    PQDBG_LVL_RETURN_VAL(zv);
}

extern void pq_ub_write(const QString &msg);
extern void pq_pre(const QString &msg, const QString &title);
//extern void pq_register_extra_zend_ce(const QString &className);
//extern void pq_register_long_constant(const QString &className, const QString &constName, int value);
//extern bool pq_register_long_constant_ex(const QString &className, const QString &constName, int value, QString ceName);
//extern void pq_register_string_constant(const QString &className, const QString &constName, const QString &value);
//extern void pq_register_string_constant_ex(const QString &className, const QString &constName, const QString &value, QString ceName);
extern void pq_php_error(const QString &error);
extern void pq_php_warning(const QString &warning);
extern void pq_php_notice(const QString &notice);

extern size_t pq_stream_reader(void *dataStreamPtr, char *buffer, size_t wantlen);
extern void pq_stream_closer(void *dataStreamPtr);
extern size_t pq_stream_fsizer(void *dataStreamPtr);

class PQDLAPI PHPQt5
{
public:
    static QByteArray       toW(const QByteArray &ba);
    static QByteArray       toUTF8(const QByteArray &ba);

    static void             pq_prepare_args(int argc, char** argv PQDBG_LVL_DC);
    static QStringList      pq_get_arguments();

    /* PHPQt5 Conversions */
    static zval             plastiq_cast_to_zval(const PMOGStackItem &stackItem);
    static zval             plastiq_cast_to_zval(const QVariant &value, const QByteArray &typeName = QByteArray());
    static void*            plastiq_cast_to_s_voidp(const PMOGStackItem &stackItem);
    static zval             plastiq_stringlist_to_array(const QStringList &list);

    static void             pq_register_basic_classes();
    static void             pq_register_plastiq_class(const PlastiQMetaObject &metaObject);
    static void             pq_qdbg_message(zval *value, zval *return_value, const QString &ftype);

    /* HANDLERS */
    static zend_object *    pqobject_create(zend_class_entry *class_type);
    static void             pqobject_object_free(zend_object *zobject);
    static void             pqobject_object_dtor(zend_object *zobject);

    static zend_object *    pqenum_create(zend_class_entry *ce);
    static void             pqenum_object_free(zend_object *zenum);
    static void             pqenum_object_dtor(zend_object *zenum);

    /* until better times :-)
    static int              pqobject_call_method(zend_string *method, zend_object *object, INTERNAL_FUNCTION_PARAMETERS);
    static zend_function *  pqobject_get_method(zend_object **zobject, zend_string *method, const zval *key);

    static zval *           pqobject_get_property_ptr_ptr(zval *object,
                                                          zval *member,
                                                          int type,
                                                          void **cache_slot);
    static HashTable *      pqobject_get_properties(zval *object);
    static zval *           pqobject_read_property(zval *object,
                                                  zval *member,
                                                  int type,
                                                  void **cache_slot,
                                                  zval *rv);
    static int              pqobject_has_property(zval *object,
                                                 zval *member,
                                                 int type,
                                                 void **cache_slot);
    */

    static bool             pq_test_ce(zval *pzval PQDBG_LVL_DC); // FIXME: return QByteArray originalClassName, to new API
    static bool             pq_declareSignal(QObject *qo, const QByteArray signalSignature); // FIXME: to new API

    static int              zm_startup_phpqt5(INIT_FUNC_ARGS);
    static void             zif_SIGNAL(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_SLOT(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_setEngineErrorHandler(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_connect(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_disconnect(INTERNAL_FUNCTION_PARAMETERS); // FIXME: to new API
    static void             zif_c(INTERNAL_FUNCTION_PARAMETERS); // ?????????
    static void             zif_tr(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_set_tr_lang(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_aboutPQ(INTERNAL_FUNCTION_PARAMETERS);

    static void             zif_pqpack(INTERNAL_FUNCTION_PARAMETERS); // FIXME: move include lines before call
    static void             zif_R(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_emit(INTERNAL_FUNCTION_PARAMETERS); // FIXME: to new API
    static void             zif_qenum(INTERNAL_FUNCTION_PARAMETERS); // FIXME: to new API

    static void             zif_qApp(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_qvariant_cast(INTERNAL_FUNCTION_PARAMETERS);

    static void             zif_qDebug(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_qWarning(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_qCritical(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_qInfo(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_qFatal(INTERNAL_FUNCTION_PARAMETERS);

    static void             zif_pqProperties(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_pqMethods(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_pqStaticFunctions(INTERNAL_FUNCTION_PARAMETERS);
    static void             zif_pqSignals(INTERNAL_FUNCTION_PARAMETERS);

    static void             zif_test_ref(INTERNAL_FUNCTION_PARAMETERS);

    /* ... */

    static PHPQt5ObjectFactory *objectFactory() {
        static PHPQt5ObjectFactory *objectFactory_instance = new PHPQt5ObjectFactory;
        return objectFactory_instance;
    }

    static PlastiQThreadCreator *threadCreator() {
        static PlastiQThreadCreator *threadCreatorInstance
                = new PlastiQThreadCreator(QThread::currentThread(),
                                           tsrm_get_ls_cache());

        return threadCreatorInstance;
    }

    static zend_function_entry *phpqt5_functions() {
        static zend_function_entry instance[] = {
            PHP_FE(SIGNAL, NULL)
            PHP_FE(SLOT, NULL)
            PHP_FE(connect, NULL)
            //PHP_FE(disconnect, NULL)
            //PHP_FE(c, NULL)
            PHP_FE(tr, NULL)
            PHP_FE(set_tr_lang, NULL)
            PHP_FE(aboutPQ, NULL)
            PHP_FE(pqpack, NULL)
            PHP_FE(R, NULL)
            PHP_FE(setEngineErrorHandler, NULL)
            PHP_FE(qenum, NULL)

            PHP_FE(pqProperties, NULL)
            PHP_FE(pqMethods, NULL)
            PHP_FE(pqStaticFunctions, NULL)
            PHP_FE(pqSignals, NULL)

            PHP_FE(test_ref, NULL)
            PHP_FE(qvariant_cast, NULL)

            { "qDebug", zif_qDebug, NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), 0 },
            { "qInfo", zif_qInfo, NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), 0 },
            { "qWarning", zif_qWarning, NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), 0 },
            { "qCritical", zif_qCritical, NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), 0 },
            { "qFatal", zif_qFatal, NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), 0 },

            { "qApp", zif_qApp, NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), 0 },
            { "emit", zif_emit, NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), 0 },
            ZEND_FE_END
        };

        return instance;
    }

    static zend_module_entry *phpqt5_module_entry() {
        static zend_module_entry instance = {
            STANDARD_MODULE_HEADER,
            "PHPQt5",
            phpqt5_functions(),
            zm_startup_phpqt5,
            NULL,
            NULL,
            NULL,
            NULL,
            PQENGINE_VERSION,
            STANDARD_MODULE_PROPERTIES
        };

        return &instance;
    }

//    static zend_function_entry *phpqt5_generic_methods() {
//        static zend_function_entry instance[] = {
//            ZEND_ME(pqobject, __construct, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, __destruct, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, __set, phpqt5__set, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, __get, phpqt5__get, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, __call, phpqt5__call, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, __toString, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, __callStatic, phpqt5__call, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
//            ZEND_ME(pqobject, qobjInfo, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, qobjProperties, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, qobjMethods, NULL, ZEND_ACC_PUBLIC)
//            // ZEND_ME(pqobject, qobjSignals, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, qobjOnSignals, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, free, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, setEventListener, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, children, NULL, ZEND_ACC_PUBLIC)
//            ZEND_ME(pqobject, declareSignal, NULL, ZEND_ACC_PUBLIC)
//            { "emit", ZEND_MN(pqobject_emit), NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), ZEND_ACC_PUBLIC },
//            ZEND_FE_END
//        };

//        return instance;
//    }

    static zend_function_entry *phpqt5_plastiq_methods() {
        static zend_function_entry plastiq_methods[] = {
            ZEND_ME(plastiq, __construct, NULL, ZEND_ACC_PUBLIC)
            ZEND_ME(plastiq, __call, phpqt5__call, ZEND_ACC_PUBLIC)
            ZEND_ME(plastiq, __callStatic, phpqt5__call, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
            ZEND_ME(plastiq, __set, phpqt5__set, ZEND_ACC_PUBLIC)
            ZEND_ME(plastiq, __get, phpqt5__get, ZEND_ACC_PUBLIC)
            ZEND_ME(plastiq, __toString, NULL, ZEND_ACC_PUBLIC)
            ZEND_ME(plastiq, free, NULL, ZEND_ACC_PUBLIC)
            ZEND_ME(plastiq, connect, NULL, ZEND_ACC_PUBLIC)
            { "emit", ZEND_MN(plastiq_emit), NULL, (uint32_t) (sizeof(NULL)/sizeof(struct _zend_internal_arg_info)-1), ZEND_ACC_PUBLIC },
            ZEND_FE_END
        };

        return plastiq_methods;
    }

    static zend_function_entry *phpqt5_qenum_methods() {
        static zend_function_entry qenum_methods[] = {
            ZEND_ME(qenum, __construct, NULL, ZEND_ACC_PUBLIC)
            ZEND_FE_END
        };

        return qenum_methods;
    }

    static zend_function_entry *phpqt5_no_methods() {
        // for pq_register_extra_zend_ce
        static zend_function_entry no_methods[] = {
            ZEND_FE_END
        };
        return no_methods;
    }

    static size_t php_qrc_read(php_stream *stream, char *buf, size_t count);
    static int php_qrc_close(php_stream *stream, int close_handle);
    static int php_qrc_flush(php_stream *stream);
    static size_t php_qrc_write(php_stream *stream, const char *buf, size_t count);
    static php_stream *qrc_opener(php_stream_wrapper *wrapper,
                    const char *path,
                    const char *mode,
                    int options,
                    zend_string **opened_path,
                    php_stream_context *context STREAMS_DC);

    static zend_object_handlers pqobject_handlers;
    static zend_object_handlers pqenum_handlers;

    static zval engineErrorHandler;

    static void             qevent_cast(PQObjectWrapper *pqobject);
    static QByteArray       plastiqGetQEventClassName(QEvent *event);
    static bool             activateConnection(PQObjectWrapper *sender, const char *signal,
                                               PQObjectWrapper *receiver, const char *slot,
                                               int argc, zval *params, bool dtor_params = false);
    static bool             doActivateConnection(PQObjectWrapper *sender, const char *signal,
                                                 PQObjectWrapper *receiver, const char *slot,
                                                 int argc, zval *params, bool dtor_params = false);

#ifdef PQDEBUG
    static QLocalSocket*      debugSocket() {
        static QLocalSocket* ds = new QLocalSocket();
        return ds;
    }
#endif


private:
    static void             pq_emit(QObject *qo, const QByteArray signalSignature, zval *args);

    // QEnum
    static void             zim_qenum___construct(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_qenum___call(INTERNAL_FUNCTION_PARAMETERS);

    // PlastiQ
    static void             zim_plastiq___construct(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq___call(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq___callStatic(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq___set(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq___get(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq___toString(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq_free(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq_connect(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq_emit(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_plastiq_testFn(INTERNAL_FUNCTION_PARAMETERS);
    static zval             plastiqCall(PQObjectWrapper *pqobject, const QByteArray &methodName, int argc, zval *argv, const PlastiQMetaObject *metaObject = Q_NULLPTR);
    static bool             plastiqConnect(zval *z_sender, const QString &signalSignature, zval *z_receiver, const QString &slotSignature, bool isOnSignal);
    static void             plastiqErrorHandler(int error_num, const char *error_filename, const uint error_lineno, const char *format, va_list args);
    // static void             plastiqEventCast(QEvent *event);

    //static void             zim_pqobject_emit(INTERNAL_FUNCTION_PARAMETERS);
    //static void             zim_pqobject_declareSignal(INTERNAL_FUNCTION_PARAMETERS);

    static void             zim_qApp___callStatic(INTERNAL_FUNCTION_PARAMETERS);

    static void             zim_qevent_ignore(INTERNAL_FUNCTION_PARAMETERS);
    static void             zim_qevent_accept(INTERNAL_FUNCTION_PARAMETERS);

    /* PRIVATE VARIABLES */
    static QByteArray W_CP;
    static QStringList mArguments;
    static QByteArray mTrLang;
    static QHash<QByteArray, QByteArray> mTrData;
};

Q_DECLARE_METATYPE(zval)
Q_DECLARE_METATYPE(zval*)

#endif // PHPQT5_H

