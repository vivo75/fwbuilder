/* 

                          Firewall Builder

                 Copyright (C) 2003 NetCitadel, LLC

  Author:  Vadim Kurland     vadim@fwbuilder.org

  $Id$

  This program is free software which we release under the GNU General Public
  License. You may redistribute and/or modify this program under the terms
  of that license as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  To get a copy of the GNU General Public License, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/


#ifndef  __OBJECTEDITOR_H_
#define  __OBJECTEDITOR_H_

#include "../../config.h"
#include "global.h"

#include "qdialog.h"

#include "fwbuilder/FWObject.h"
#include "fwbuilder/FWObjectDatabase.h"

#include <qmap.h>
#include <qstring.h>

class ObjectTreeViewItem;
class QComboBox;
class QMenu;
class QStackedWidget;

class ProjectPanel;
class BaseObjectDialog;

class ObjectEditor : public QObject {

    Q_OBJECT;

    QMap<QString, int> stackIds;
    QMap<int, BaseObjectDialog*> dialogs;
    libfwbuilder::FWObject *opened;
    int current_dialog_idx;
    QString current_dialog_name;
    QStackedWidget *editorStack;
    QPushButton *helpButton;
    ProjectPanel *m_project;
    
    void disconnectSignals();
    void findAndLoadHelp();

public: 
    enum OptType{optAction, optComment, optMetric, optNone, optRuleCompile};

private: 
   OptType  openedOpt;

    void registerObjectDialog(QStackedWidget *stack,
                              const QString &obj_type,
                              const QString &dialog_name);
    void registerOptDialog(QStackedWidget *stack,
                           ObjectEditor::OptType opt_type,
                           const QString &dialog_name);
    void activateDialog(const QString &dialog_name,
                        libfwbuilder::FWObject *obj,
                        enum OptType opt);

public:
   
    ObjectEditor(QWidget *parent);
    virtual ~ObjectEditor() {}

    void attachToProjectWindow(ProjectPanel *pp);

    QString getOptDialogName(OptType t);
    void open(libfwbuilder::FWObject *o);
    void openOpt(libfwbuilder::FWObject *, OptType t);
    void show();
    void hide();
    bool isVisible();
    bool isModified();

    libfwbuilder::FWObject* getOpened() { return opened; };
    OptType getOpenedOpt() {return openedOpt;};

    void purge();

    void setHelpButton(QPushButton * b);
    void selectObject(libfwbuilder::FWObject *o);
    void selectionChanged(libfwbuilder::FWObject *o);
    void actionChanged(libfwbuilder::FWObject *o);

    int getCurrentDialogIndex() { return current_dialog_idx; };
    QWidget* getCurrentObjectDialog();
    
public slots:
    void apply();
    void help();
    //void discard();
    void changed();
    void blank();

    /*
     * this slot is used by the object dialog class to notify
     * ObjectEditor that it applied changes to the object. This way,
     * object dialog can call its applyChanges() method itself, not
     * only when user clicks "Apply" button.  Slot
     * notifyChangesApplied() attached to this signal can disable
     * Apply button and do other things as appropriate.
     */
    void notifyChangesApplied();

signals:

    /**
     * the dialog class should have a slot that can load object's data
     * into dialog elements when ObjectEditor emits this signal
     */
    void loadObject_sign(libfwbuilder::FWObject *);

    /**
     * the dialog class should have a slot that can verify data entered by
     * user in the dialog elements when ObjectEditor emits this
     * signal. The validation result is returned in variable "bool *res"
     */
    void validate_sign(bool *res);

    /**
     * the dialog class should have a slot that can verify if the data in
     * dialog has been edited.
     */
    void isChanged_sign(bool *res);

    /**
     * the dialog class should have a slot that applies changes made by
     * the user and saves data in the object.
     */
    void applyChanges_sign();

    /**
     * the dialog class should have a slot that discards changes made by
     * the user 
     */
    void discardChanges_sign();

    /**
     * the dialog class returns corresponding help file name
     */
    void getHelpName_sign(QString *str);
};

#endif
