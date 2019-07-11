/****************************************************************************
** Meta object code from reading C++ file 'GoalkeeperAnalysis.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.9.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../src/GoalkeeperAnalysis.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GoalkeeperAnalysis.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.9.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_GoalkeeperAnalysis_t {
    QByteArrayData data[17];
    char stringdata0[331];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_GoalkeeperAnalysis_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_GoalkeeperAnalysis_t qt_meta_stringdata_GoalkeeperAnalysis = {
    {
QT_MOC_LITERAL(0, 0, 18), // "GoalkeeperAnalysis"
QT_MOC_LITERAL(1, 19, 20), // "userDirectoryChanged"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 4), // "text"
QT_MOC_LITERAL(4, 46, 19), // "calibrationFinished"
QT_MOC_LITERAL(5, 66, 27), // "calibrationAccuracyFinished"
QT_MOC_LITERAL(6, 94, 21), // "liveGazeDisplayClosed"
QT_MOC_LITERAL(7, 116, 19), // "startCaptureSession"
QT_MOC_LITERAL(8, 136, 16), // "startCalibration"
QT_MOC_LITERAL(9, 153, 19), // "startSavingEyevideo"
QT_MOC_LITERAL(10, 173, 34), // "startEvaluatingCalibrationAcc..."
QT_MOC_LITERAL(11, 208, 20), // "startPlaybackSession"
QT_MOC_LITERAL(12, 229, 16), // "startLiveTesting"
QT_MOC_LITERAL(13, 246, 21), // "startCaptureFullImage"
QT_MOC_LITERAL(14, 268, 14), // "stopAllOngoing"
QT_MOC_LITERAL(15, 283, 20), // "refreshProfilingInfo"
QT_MOC_LITERAL(16, 304, 26) // "resetGazeEstimationFilters"

    },
    "GoalkeeperAnalysis\0userDirectoryChanged\0"
    "\0text\0calibrationFinished\0"
    "calibrationAccuracyFinished\0"
    "liveGazeDisplayClosed\0startCaptureSession\0"
    "startCalibration\0startSavingEyevideo\0"
    "startEvaluatingCalibrationAccuracy\0"
    "startPlaybackSession\0startLiveTesting\0"
    "startCaptureFullImage\0stopAllOngoing\0"
    "refreshProfilingInfo\0resetGazeEstimationFilters"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_GoalkeeperAnalysis[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x0a /* Public */,
       4,    0,   87,    2, 0x0a /* Public */,
       5,    0,   88,    2, 0x0a /* Public */,
       6,    0,   89,    2, 0x0a /* Public */,
       7,    0,   90,    2, 0x08 /* Private */,
       8,    0,   91,    2, 0x08 /* Private */,
       9,    0,   92,    2, 0x08 /* Private */,
      10,    0,   93,    2, 0x08 /* Private */,
      11,    0,   94,    2, 0x08 /* Private */,
      12,    0,   95,    2, 0x08 /* Private */,
      13,    0,   96,    2, 0x08 /* Private */,
      14,    0,   97,    2, 0x08 /* Private */,
      15,    0,   98,    2, 0x08 /* Private */,
      16,    0,   99,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void GoalkeeperAnalysis::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GoalkeeperAnalysis *_t = static_cast<GoalkeeperAnalysis *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->userDirectoryChanged((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->calibrationFinished(); break;
        case 2: _t->calibrationAccuracyFinished(); break;
        case 3: _t->liveGazeDisplayClosed(); break;
        case 4: _t->startCaptureSession(); break;
        case 5: _t->startCalibration(); break;
        case 6: _t->startSavingEyevideo(); break;
        case 7: _t->startEvaluatingCalibrationAccuracy(); break;
        case 8: _t->startPlaybackSession(); break;
        case 9: _t->startLiveTesting(); break;
        case 10: _t->startCaptureFullImage(); break;
        case 11: _t->stopAllOngoing(); break;
        case 12: _t->refreshProfilingInfo(); break;
        case 13: _t->resetGazeEstimationFilters(); break;
        default: ;
        }
    }
}

const QMetaObject GoalkeeperAnalysis::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_GoalkeeperAnalysis.data,
      qt_meta_data_GoalkeeperAnalysis,  qt_static_metacall, nullptr, nullptr}
};


const QMetaObject *GoalkeeperAnalysis::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *GoalkeeperAnalysis::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_GoalkeeperAnalysis.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int GoalkeeperAnalysis::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
