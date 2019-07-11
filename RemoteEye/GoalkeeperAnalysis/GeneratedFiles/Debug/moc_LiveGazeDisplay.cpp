/****************************************************************************
** Meta object code from reading C++ file 'LiveGazeDisplay.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/LiveGazeDisplay.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'LiveGazeDisplay.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LiveGazeDisplay_t {
    QByteArrayData data[7];
    char stringdata0[66];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LiveGazeDisplay_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LiveGazeDisplay_t qt_meta_stringdata_LiveGazeDisplay = {
    {
QT_MOC_LITERAL(0, 0, 15), // "LiveGazeDisplay"
QT_MOC_LITERAL(1, 16, 5), // "ended"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 7), // "setGaze"
QT_MOC_LITERAL(4, 31, 9), // "gaze_left"
QT_MOC_LITERAL(5, 41, 10), // "gaze_right"
QT_MOC_LITERAL(6, 52, 13) // "gaze_combined"

    },
    "LiveGazeDisplay\0ended\0\0setGaze\0gaze_left\0"
    "gaze_right\0gaze_combined"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LiveGazeDisplay[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    3,   25,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::QPoint, QMetaType::QPoint, QMetaType::QPoint,    4,    5,    6,

       0        // eod
};

void LiveGazeDisplay::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        LiveGazeDisplay *_t = static_cast<LiveGazeDisplay *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ended(); break;
        case 1: _t->setGaze((*reinterpret_cast< QPoint(*)>(_a[1])),(*reinterpret_cast< QPoint(*)>(_a[2])),(*reinterpret_cast< QPoint(*)>(_a[3]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (LiveGazeDisplay::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LiveGazeDisplay::ended)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject LiveGazeDisplay::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_LiveGazeDisplay.data,
      qt_meta_data_LiveGazeDisplay,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *LiveGazeDisplay::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LiveGazeDisplay::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_LiveGazeDisplay.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int LiveGazeDisplay::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void LiveGazeDisplay::ended()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
