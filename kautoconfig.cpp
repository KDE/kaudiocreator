#include "kautoconfig.h"
#include <qobjectlist.h> 
#include <kconfig.h>

#include <qptrlist.h>
#include <qmap.h>
#include <qdict.h> 
#include <qcolor.h>

#include <qcheckbox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <kcolorbutton.h>

class KAutoConfig::KAutoConfigPrivate {
public:
  KAutoConfigPrivate() : retrieved(false), knownWidgets(5, true) { }
  QPtrList<QWidget> widgets;
  QPtrList<QWidget> ignore;
  QMap<QWidget*, QString> groups;
  
  bool retrieved;
  
  QDict<int> knownWidgets; 

  QMap<QObject*, int> numDefault;
  QMap<QObject*, bool> boolDefault;
  QMap<QObject*, QString> stringDefault;
  QMap<QObject*, QColor> colorDefault;
};

#define RESTORE 000
#define SAVE    100
#define RESET   200

#define retrievedError "KAutoConfig can't handle save/reset settings because it hasn't retrieved yet.  Action canceled.  Please fix the program."

KAutoConfig::KAutoConfig(KConfig *kconfig) : config(kconfig) {
  d = new KAutoConfigPrivate();
  d->knownWidgets.insert("QCheckBox",    new int(1));
  d->knownWidgets.insert("QSpinBox",     new int(2));
  d->knownWidgets.insert("QLineEdit",    new int(3));
  d->knownWidgets.insert("QComboBox",    new int(4));
  d->knownWidgets.insert("QRadioButton", new int(5));
  d->knownWidgets.insert("KColorButton", new int(6));
  d->knownWidgets.setAutoDelete(true);
};

KAutoConfig::~KAutoConfig(){
  delete d;
}

void KAutoConfig::addWidget(QWidget *widget, QString group){
  d->widgets.insert(0,widget);
  d->groups.insert(widget, group);
}

void KAutoConfig::ignoreSubWidget(QWidget *widget){
  d->ignore.insert(0,widget);
}

void KAutoConfig::retrieveSettings(){
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    parseWidget(widget, RESTORE);
  }
  d->retrieved = true;
}

bool KAutoConfig::saveSettings() {
  if(!d->retrieved){
    qDebug(retrievedError);
    return false;
  } 
  bool changed = false;
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    config->setGroup(d->groups[widget]);
    changed = changed || parseWidget(widget, SAVE);
  }
  return changed;
}

void KAutoConfig::resetSettings(){
  if(!d->retrieved){
    qDebug(retrievedError);
    return;
  }
  QPtrListIterator<QWidget> it( d->widgets );
  QWidget *widget;
  while ( (widget = it.current()) != 0 ) {
    ++it;
    parseWidget(widget, RESET);
  }
}

	
#define objectReadWrite(id, defaultMap, type, typeread, typewrite, confread )\
      case id : \
	if(defaultMap.find(object) == defaultMap.end()) \
          defaultMap.insert(object, ((type*)object)->typeread()); \
        ((type*)object)->typewrite(config->confread(object->name(), defaultMap[object])); \
      break; \
      case 10##id : \
        if(config->confread(object->name(), defaultMap[object]) != ((type*)object)->typeread()){ \
	  config->writeEntry(object->name(), ((type*)object)->typeread()); \
	  changed = true; \
	} \
      break; \
      case 20##id : \
        ((type*)object)->typewrite(defaultMap[object]); \
      break;
//out = QString("Group:%1, Object:%2, Name:%3, Value:%4").arg(group).arg(object->className()).arg(object->name()).arg(((type*)object)->typeread()); 
//	  qDebug("The Value has changed of %s", out.latin1()); 
	  
bool KAutoConfig::parseWidget(QWidget *widget, int op){
  bool changed = false;
  QString group = config->group(); 
  const QPtrList<QObject> *list = widget->children();
  if(list == 0)
    return changed;
  
  QPtrListIterator<QObject> it( *list );
  QObject *object;
  while ( (object = it.current()) != 0 ) {
    ++it;
    if(!object->isWidgetType())
      continue;
    QWidget *w = (QWidget *)object;
    if(d->ignore.containsRef(w))
      continue;
	   
    int objectType = 0;
    if(d->knownWidgets[object->className()] != NULL)
      objectType = *d->knownWidgets[object->className()];
   
    // If type statment for reading or writing the object
    objectType ^= op;

    QString out;
    switch(objectType){
      objectReadWrite(1, d->boolDefault, QCheckBox, isChecked, setChecked, readBoolEntry );
      objectReadWrite(2, d->numDefault, QSpinBox, value, setValue, readNumEntry );
      objectReadWrite(3, d->stringDefault, QLineEdit, text, setText, readEntry );
      objectReadWrite(4, d->numDefault, QComboBox, currentItem, setCurrentItem, readNumEntry );
     

      // QRadioButton has its own read/write because it is stored by the parent name.
      case 5: // QRadioButton read
	if(d->boolDefault.find(object) == d->boolDefault.end())
          d->boolDefault.insert(object, ((QRadioButton*)object)->isChecked());
        ((QRadioButton*)object)->setChecked(config->readBoolEntry(object->parent()->name(), d->boolDefault[object]));
      break;
      case 105: // QRadioButton write
        if(config->readNumEntry(object->name(), d->boolDefault[object]) != ((QRadioButton*)object)->isChecked() && ((QRadioButton*)object)->isChecked()){
          out = QString("Group:%1, Object:%2, Name:%3, Value:%4").arg(group).arg(object->parent()->className()).arg(object->parent()->name()).arg(((QRadioButton*)object)->isChecked());
	  qDebug("The Value has changed of %s", out.latin1());
	  config->writeEntry(object->parent()->name(), ((QRadioButton*)object)->isChecked());
	  changed = true;
	}
      break;
      case 205 : 
        ((QRadioButton*)object)->setChecked(d->boolDefault[object]);
      break;
      
      //KColorButton  has its own read/write because it is stores a pointer.
      case 6: // KColorButton read
	if(d->colorDefault.find(object) == d->colorDefault.end())
          d->colorDefault.insert(object, ((KColorButton*)object)->color());
        ((KColorButton*)object)->setColor(config->readBoolEntry(object->parent()->name(), &d->colorDefault[object]));
      break;
      case 106: // KColorButton write
        if(config->readColorEntry(object->name(), &d->colorDefault[object]) != ((KColorButton*)object)->color()){
	  config->writeEntry(object->parent()->name(), ((KColorButton*)object)->color());
	  changed = true;
	}
      break;
      case 207 : 
        ((KColorButton*)object)->setColor(d->colorDefault[object]);
      break;
      
      default: // Unknown
        //qDebug("Unknown Object %s", object->className());
	changed = changed || parseWidget(w, op);
	continue;
      break;
    }
  }
  return changed;
}

#undef objectReadWrite

