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

#include "../../config.h"
#include "global.h"
#include "utils.h"

#include "IPv4Dialog.h"

#include "ProjectPanel.h"
#include "fwbuilder/Library.h"
#include "fwbuilder/IPv4.h"
#include "fwbuilder/Interface.h"
#include "fwbuilder/FWException.h"

#include <qlineedit.h>
#include <qspinbox.h>
#include <qcheckbox.h>
#include <qtextedit.h>
#include <qcombobox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qhostinfo.h>
#include <qapplication.h>
#include <qcursor.h>
#include "FWBSettings.h"

#include <iostream>

#include "FWWindow.h"

using namespace std;
using namespace libfwbuilder;

IPv4Dialog::IPv4Dialog(ProjectPanel *project, QWidget *parent) : QWidget(parent), m_project(project)
{
    m_dialog = new Ui::IPv4Dialog_q;
    m_dialog->setupUi(this);

    obj=NULL;
}

IPv4Dialog::~IPv4Dialog()
{
    delete m_dialog;
}

void IPv4Dialog::loadFWObject(FWObject *o)
{
    obj=o;
    IPv4 *s = dynamic_cast<IPv4*>(obj);
    assert(s!=NULL);

    dnsBusy=false;
    init=true;

    m_dialog->obj_name->setText( QString::fromUtf8(s->getName().c_str()) );
    m_dialog->comment->setText( QString::fromUtf8(s->getComment().c_str()) );

/*
 * if this is an address that belongs to an interface, we can't move
 * it from library to library just like that. Only IPv4 objects that
 * belong to the standard group "Addresses" can be moved.
 */
    if ( Interface::isA( obj->getParent() ) )
    {
        showNetmask=true;
        m_dialog->netmaskLabel->show();
        m_dialog->netmask->show();
    } else
    {
        showNetmask=false;
        m_dialog->netmaskLabel->hide();
        m_dialog->netmask->hide();
    }

/* catch exceptions separately so even if we have a bad address, we
 * still can show netmask */
    try
    {
        m_dialog->address->setText( s->getAddressPtr()->toString().c_str() );
    } catch (FWException &ex) {}

    try
    {
        if ( Interface::isA( obj->getParent() ) )
            m_dialog->netmask->setText( s->getNetmaskPtr()->toString().c_str() );
    } catch (FWException &ex) {}

    //apply->setEnabled( false );

    m_dialog->obj_name->setEnabled(!o->isReadOnly());
    setDisabledPalette(m_dialog->obj_name);

    m_dialog->address->setEnabled(!o->isReadOnly());
    setDisabledPalette(m_dialog->address);

    m_dialog->netmask->setEnabled(!o->isReadOnly());
    setDisabledPalette(m_dialog->netmask);

    m_dialog->comment->setReadOnly(o->isReadOnly());
    setDisabledPalette(m_dialog->comment);



    init=false;
}

void IPv4Dialog::changed()
{
    //apply->setEnabled( true );
    emit changed_sign();
}

void IPv4Dialog::validate(bool *res)
{
    *res=true;

    if (!isTreeReadWrite(this,obj)) { *res=false; return; }
    if (!validateName(this,obj,m_dialog->obj_name->text())) { *res=false; return; }

    IPv4 *s = dynamic_cast<IPv4*>(obj);
    assert(s!=NULL);
    try
    {
        InetAddr( m_dialog->address->text().toLatin1().constData() );
    } catch (FWException &ex)
    {
        *res=false;
        QMessageBox::critical(this, "Firewall Builder",
                              tr("Illegal IP address '%1'").arg(m_dialog->address->text()),
                              tr("&Continue"), 0, 0,
                              0 );
    }

    if ( showNetmask )
    {
        try
        {
            InetAddr( m_dialog->netmask->text().toLatin1().constData() );
        } catch (FWException &ex)
        {
            *res=false;
            QMessageBox::critical(this, "Firewall Builder",
                                  tr("Illegal netmask '%1'").arg(m_dialog->netmask->text()),
                                  tr("&Continue"), 0, 0,
                                  0 );
        }
    }
}

void IPv4Dialog::isChanged(bool *)
{
    //*res=(!init && apply->isEnabled());

}

void IPv4Dialog::libChanged()
{
    changed();
}

void IPv4Dialog::applyChanges()
{
    IPv4 *s = dynamic_cast<IPv4*>(obj);
    assert(s!=NULL);

    string oldname=obj->getName();
    obj->setName( string(m_dialog->obj_name->text().toUtf8().constData()) );
    obj->setComment( string(m_dialog->comment->toPlainText().toUtf8().constData()) );

    try
    {
        s->setAddress(
            InetAddr(m_dialog->address->text().toLatin1().constData()) );
    } catch (FWException &ex) { }

    if ( showNetmask )
    {
        try
        {
            s->setNetmask(
                InetAddr(m_dialog->netmask->text().toLatin1().constData()) );
        } catch (FWException &ex) { }
    } else
        s->setNetmask(InetAddr());

    mw->updateObjName(obj,QString::fromUtf8(oldname.c_str()));

    //apply->setEnabled( false );
    mw->updateLastModifiedTimestampForAllFirewalls(obj);
}

void IPv4Dialog::discardChanges()
{
    loadFWObject(obj);
}

void IPv4Dialog::DNSlookup()
{
    if (fwbdebug)
        qDebug("IPv4Dialog::DNSlookup()  dnsBusy=%d", dnsBusy);

    if (!dnsBusy)
    {
        QString name = m_dialog->obj_name->text();
        if (fwbdebug) qDebug("IPv4Dialog::DNSlookup()  name=%s", name.toAscii().constData());
        dnsBusy=true;
        QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
        QString addr = getAddrByName(name);
        QApplication::restoreOverrideCursor();
        dnsBusy=false;
        if (fwbdebug) qDebug("IPv4Dialog::DNSlookup()  done");

        if (! addr.isEmpty())
        {
            m_dialog->address->setText( addr );
            return;
        }

        if ( Interface::isA(obj->getParent()) )
        {
            FWObject *host = obj->getParent()->getParent();
            assert(host!=NULL);
            name = host->getName().c_str();

            if (fwbdebug) qDebug("IPv4Dialog::DNSlookup()  name=%s",
                name.toAscii().constData());
            dnsBusy=true;
            QApplication::setOverrideCursor( QCursor( Qt::WaitCursor) );
            QString addr = getAddrByName(name);
            QApplication::restoreOverrideCursor();
            dnsBusy=false;
            if (fwbdebug) qDebug("IPv4Dialog::DNSlookup()  done");
            if ( ! addr.isEmpty())
            {
                m_dialog->address->setText( addr );
                return;
            }
            QMessageBox::warning(
                this,"Firewall Builder",
                tr("DNS lookup failed for both names of the address object '%1' and the name of the host '%2'.")
                .arg(m_dialog->obj_name->text()).arg(name),
                "&Continue", QString::null,QString::null, 0, 1 );
            return;
        }
        QMessageBox::warning(
            this,"Firewall Builder",
            tr("DNS lookup failed for name of the address object '%1'.")
            .arg(name),
            "&Continue", QString::null,QString::null, 0, 1 );
        return;
    }
}

void IPv4Dialog::closeEvent(QCloseEvent *e)
{
    if (fwbdebug)
        qDebug("IPv4Dialog::closeEvent  got close event: %p",e);
    emit close_sign(e);
}

