#ifndef FLISTFRAME_H
#define FLISTFRAME_H

#include "fwidgets.h"
#include "fstring.h"
#include <QToolButton>
#include <QFileSystemModel>


class FListFrame : public QWidget//: public common
{
Q_OBJECT


private:

  inline void updateIndexInfo();
#if 0
  void deleteGroups(QList<int> &L);
#endif

 QList<QListWidget*> widgetContainer;
 FListWidget* fileListWidget;
 QString frameHashKey;

 void addGroup();

 int row, currentIndex,  slotListSize;

public:

 QToolButton *importFromMainTree=new QToolButton,
                        *moveDownItemButton=new QToolButton,
                        *moveUpItemButton=new QToolButton,
                        *retrieveItemButton=new QToolButton,
                        *clearListButton=new QToolButton,
                        *addGroupButton=new QToolButton,
                        *deleteGroupButton=new QToolButton;

 QTabWidget *mainTabWidget, *embeddingTabWidget;
 QAbstractItemView *fileTreeView;
 QStringList* slotList= new QStringList;
 QStringList *signalList= new QStringList;
 QList<int> cumulativePicCount;
 QLabel* fileLabel=new QLabel;
 QString fileLabelText;
 QFileSystemModel *model=new QFileSystemModel;
 QGroupBox *controlButtonBox=new QGroupBox, *tabBox=new QGroupBox;

 /* accessors */
 int getRank() {return widgetContainer.count()-1;}
 const QString &getHashKey() const {return frameHashKey;}
 void initializeWidgetContainer()
 {
    widgetContainer = QList<QListWidget*>() << fileListWidget->currentListWidget;
 }
 void clearWidgetContainer()
 {
    widgetContainer.clear(); ;
 }

inline QList<QListWidget*>  getWidgetContainer() {return widgetContainer;}
inline QListWidget*  getWidgetContainer(int rank) {if (rank < widgetContainer.count()) return widgetContainer[rank]; else return nullptr;}
inline QListWidget*  getCurrentWidget() { return widgetContainer[this->mainTabWidget->currentIndex()];}

inline int getCurrentIndex() {return this->mainTabWidget->currentIndex();}
inline int getCurrentRow() {return getCurrentWidget()->currentRow();}

void addDirectoryToListWidget(const QFileInfo&, int);
void addNewTab();
bool addStringToListWidget(const QString& , int );


FListFrame(QObject* parent,  QAbstractItemView * fileTreeView, short import_type, const QString &hashKey,
            const QStringList &description, const QString &command_line, int commandLineType, const QStringList &separator, const QStringList &xml_tags,
            int mainTabWidgetRank=-1, QIcon* icon=NULL, QTabWidget* parentTabWidget=NULL,
           QStringList* terms=NULL, QStringList* translation=NULL, QStringList* slotL=NULL);


public slots:

    void deleteGroup();
    void on_deleteItem_clicked();
     void on_clearList_clicked(int currentIndex=-1);

protected slots:

    void on_importFromMainTree_clicked();
    void on_moveDownItemButton_clicked();
    void on_moveUpItemButton_clicked();
    void addGroups(int);
    void on_mainTabIndex_changed(int =0);
    void on_embeddingTabIndex_changed(int =0);

protected:
    short importType;
    QStringList tags;

signals:
    void is_signalList_changed(int);

};

inline void FListFrame::updateIndexInfo()
{
  fileListWidget->currentListWidget=qobject_cast<QListWidget*>(mainTabWidget->currentWidget());
  if (fileListWidget->currentListWidget == NULL) return;
  row=fileListWidget->currentListWidget->currentRow();
  currentIndex=mainTabWidget->currentIndex();
}


#endif // FLISTFRAME_H