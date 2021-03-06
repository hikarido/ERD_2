#include "widget.h"

Widget::Widget(QWidget *parent)
	: QWidget(parent)
{
	this->layoutH = new QHBoxLayout();
	this->layoutV = new QVBoxLayout();
	this->erView = new ERGraphicsView();
	this->treeModel = new TreeModel();
	this->treeViev = new QTreeView();
	this->dataView = new DataModelGraphicsView();
	this->treeModelForOneEssence = new TreeModelForOneEssence("__EMPTY__");
	this->treeVievOneEssence = new QTreeView();
	this->bar = new QMenuBar();
	this->tabs = new QTabWidget();

	this->vSplit = new QSplitter(Qt::Vertical);
	this->hSplit = new QSplitter(Qt::Horizontal);
	createWindow();

	initBar();
	setUpTreeViews();

	EssenceGraphicsController::instance();
	EssenceGraphicsController::setSceneToErModel(erView->getScene());
	EssenceGraphicsController::setSceneToDataModel(dataView->getScene());

    setUpSignalsAndSlots();
	setUpConsoleProperty();

}

bool Widget::setUpSignalsAndSlots()
{
	QObject::connect(this->erView, SIGNAL(doCreation(int)), this, SLOT(createEssence(int)));
	QObject::connect(this->erView, SIGNAL(doRelationOperation(int)), this, SLOT(performRelationOperation(int)));
	QObject::connect(EssenceGraphicsController::instance(), SIGNAL(startEditEssence(QString)), this, SLOT(editEssence(QString)));
	QObject::connect(EssenceGraphicsController::instance(), SIGNAL(startFocucedEssence(QString)), this, SLOT(treeViewEssenceUpdate(QString)));
	QShortcut * key = new QShortcut(QKeySequence("Ctrl+S"), this);
	key->setContext(Qt::ShortcutContext::ApplicationShortcut);
	QObject::connect(key, SIGNAL(activated()), this, SLOT(quick_save()));
}

bool Widget::initBar()
{


	QAction * newAct1 = new QAction(tr("&New"), this);
	connect(newAct1, SIGNAL(triggered(bool)), this, SLOT(new_file()));

	QAction * newAct2 = new QAction(tr("&Open"), this);
	connect(newAct2, SIGNAL(triggered(bool)), this, SLOT(load_file()));

	QAction * newAct3 = new QAction(tr("&Save as"), this);
	connect(newAct3, SIGNAL(triggered(bool)), this, SLOT(save_as()));

	QAction * newAct5 = new QAction(tr("&Exit"), this);
	connect(newAct5, SIGNAL(triggered(bool)), this, SLOT(endWork()));

	bar->addAction(newAct1);
	bar->addAction(newAct2);
	bar->addAction(newAct3);
	bar->addAction(newAct5);

}

void Widget::createWindow()
{
	setGeometry(0,0, 1200, 900);

	QWidget * test_w1 = new QWidget;
	QHBoxLayout * test_lay1 = new QHBoxLayout;
	test_lay1->addWidget(erView);

	test_w1->setLayout(test_lay1);
	tabs->addTab(test_w1, "ER model");

	QWidget * test_w2 = new QWidget;
	QHBoxLayout * test_lay2 = new QHBoxLayout;
	test_lay2->addWidget(dataView);

	test_w2->setLayout(test_lay2);
	tabs->addTab(test_w2, "Data model");

	setWindowFlags( ( (this->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint));

	treeVievOneEssence->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	hSplit->addWidget(treeViev);
	hSplit->addWidget(tabs);
	hSplit->addWidget(treeVievOneEssence);
	layoutH->addWidget(hSplit);

	layoutV->addWidget(bar);

	QWidget * hComponents = new QWidget;
	hComponents->setLayout(layoutH);

	vSplit->addWidget(hComponents);
	vSplit->addWidget(ConsoleOutput::getBrowser());

	layoutV->addWidget(vSplit);

	this->setLayout(layoutV);
}

void Widget::setUpConsoleProperty()
{

	ConsoleOutput::getInstance()->printSystemMassage("Hello, I'am ERD. Good luck");

}

Widget::~Widget()
{

}

void Widget::syncTreeViev()
{
	delete treeModel;
	treeModel = new TreeModel;
	treeViev->setModel(treeModel);
	//	treeViev->expandAll();
}

void Widget::setUpTreeViews()
{
	treeVievOneEssence->setMinimumWidth(100);
	treeVievOneEssence->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);

	treeViev->setGeometry(QRect(0,0, 10, 100));
	treeViev->setMinimumWidth(100);
	treeViev->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Expanding);

	treeViev->setModel(treeModel);
	treeVievOneEssence->setModel(treeModelForOneEssence);



	treeVievOneEssence->setRootIsDecorated(false);
	treeVievOneEssence->setAlternatingRowColors(true);
	treeVievOneEssence->setAllColumnsShowFocus(true);
	treeVievOneEssence->setExpandsOnDoubleClick(false);
	treeVievOneEssence->setEditTriggers(QAbstractItemView::NoEditTriggers);
	treeVievOneEssence->setSortingEnabled(true);
	treeVievOneEssence->sortByColumn(0, Qt::AscendingOrder);
	treeVievOneEssence->header()->resizeSections(QHeaderView::ResizeToContents);
	treeViev->setAlternatingRowColors(true);
}

void Widget::editEssence(QString id)
{
//	qDebug() <<"in edit";
	int type = DataController::getInstance()->search(id)->getType();
	EssenceCreateWindow * win = new EssenceCreateWindow(id, type);
//	QObject::connect(win, SIGNAL(toDeleteEssence(QString)), this, SLOT(removeEssence(QString)));
	QObject::connect(win, SIGNAL(endSuccessCreation(QString)), this, SLOT(addEssenceOnScreen(QString)));
	QObject::connect(win, SIGNAL(endSuccessEditation(QString)), this, SLOT(addEssenceOnScreen(QString)));
	QObject::connect(win, SIGNAL(endDeletetion(QString)), this, SLOT(removeEssence(QString)));
	QObject::connect(win, SIGNAL(exitFromCreateWindow()), this, SLOT(closeEditWindow()));
	this->setEnabled(false);

	win->show();
}

void Widget::createEssence(int type)
{

	EssenceCreateWindow * win = new EssenceCreateWindow("", type);
	QObject::connect(win, SIGNAL(endSuccessCreation(QString)), this, SLOT(addEssenceOnScreen(QString)));
	QObject::connect(win, SIGNAL(exitFromCreateWindow()), this, SLOT(closeEditWindow()));
	this->setEnabled(false);
	win->show();

}

void Widget::addEssenceOnScreen(QString id)
{
	EREssenceData * data = DataController::getInstance()->search(id);

	if(data == nullptr)
	{
		ConsoleOutput::getInstance()->printSystemError("__ERROR__: in void Widget::addEssenceOnScreen(QString id) nullptr expection");
		return;
	}

	EssenceGraphicsController::instance()->syncWithDataContriller();
	syncTreeViev();
	this->setEnabled(true);

}

void Widget::removeEssence(QString id)
{
	EssenceGraphicsController::instance()->syncWithDataContriller();
	syncTreeViev();
	this->setEnabled(true);
}

void Widget::performRelationOperation(int action_code)
{
	switch (action_code)
	{
		case 3:
		{
			RelationOperationWindow * w = new RelationOperationWindow(action_code);
			QObject::connect(w, SIGNAL(successRelationOperation(bool)), this, SLOT(afterPerformRelationOperation(bool)));
			QObject::connect(w, SIGNAL(cancelRelationOperation(bool)), this, SLOT(closeEditWindow()));
			this->setEnabled(false);
			w->show();
			//qDebug("Создаю: связь");
			break;
		}
		case 4:
		{
			RelationOperationWindow * w = new RelationOperationWindow(action_code);
			QObject::connect(w, SIGNAL(successRelationOperation(bool)), this, SLOT(afterPerformRelationOperation(bool)));
			QObject::connect(w, SIGNAL(cancelRelationOperation(bool)), this, SLOT(closeEditWindow()));
			this->setEnabled(false);
			w->show();
			break;

		}
		case 5:
		{
			RelationOperationWindow * w = new RelationOperationWindow(action_code);
			QObject::connect(w, SIGNAL(successRelationOperation(bool)), this, SLOT(afterPerformRelationOperation(bool)));
			QObject::connect(w, SIGNAL(cancelRelationOperation(bool)), this, SLOT(closeEditWindow()));
			this->setEnabled(false);
			w->show();
			break;
		}
		case 6:
		{
			RelationOperationWindow * w = new RelationOperationWindow(action_code);
			QObject::connect(w, SIGNAL(successRelationOperation(bool)), this, SLOT(afterPerformRelationOperation(bool)));
			QObject::connect(w, SIGNAL(cancelRelationOperation(bool)), this, SLOT(closeEditWindow()));
			this->setEnabled(false);
			w->show();
			//qDebug("изменяю кардинальность");
			break;

		}
		default:
			ConsoleOutput::getInstance()->printSystemError("__ERROR_: in void Widget::performRelationOperation(int action_code) error action code");
			break;
	}

}

void Widget::afterPerformRelationOperation(bool test)
{
	EssenceGraphicsController::instance()->syncWithDataContriller();
	syncTreeViev();
	this->setEnabled(true);
}

void Widget::quick_save()
{
//	qDebug() << "quick save";
    if(DataController::getInstance()->saveIsPermit() == true)
    {
        DataController::getInstance()->saveState("");
        ConsoleOutput::getInstance()->printInfo("Сохранено в quick_save.json");
        return;
    }

    ConsoleOutput::getInstance()->printInfo("Не могу сохранить файл");
}

void Widget::save_as()
{
//	qDebug() << "Save as";
	QString fileName = QFileDialog::getSaveFileName(this,
			tr("ER_Diagram"), "",
			tr("ER Diagram (*.json);;All Files (*)"));

//	qDebug() << fileName;

    if(DataController::getInstance()->saveIsPermit() == true)
    {
        DataController::getInstance()->saveState(fileName);
        ConsoleOutput::getInstance()->printInfo("Сохранено");
        return;
    }

    ConsoleOutput::getInstance()->printInfo("Не могу сохранить файл");
}

void Widget::new_file()
{
//	qDebug() << "new file";
	DataController::getInstance()->clear();
	syncTreeViev();
	EssenceGraphicsController::instance()->syncWithDataContriller();

}

void Widget::load_file()
{
	ConsoleOutput::getInstance()->printSystemMassage("load file");
	QString fileName = QFileDialog::getOpenFileName(this,
			tr("ER_Diagram"), "",
			tr("ER Diagram (*.json);;All Files (*)"));
//	qDebug() << fileName;
	DataController::getInstance()->loadState(fileName);
	syncTreeViev();
	EssenceGraphicsController::instance()->syncWithDataContriller();


}

void Widget::endWork()
{
	ConsoleOutput::getInstance()->printSystemMassage("End of Work");
	ConsoleOutput::getInstance()->printSystemMassage("save before exit");
	DataController::getInstance()->saveState("");
	close();
}

void Widget::help()
{
//	qDebug() << "help";
}

void Widget::treeViewEssenceUpdate(QString id)
{
//	qDebug() << "Updating";
	delete treeModelForOneEssence;
	treeModelForOneEssence = new TreeModelForOneEssence(id);
	treeVievOneEssence->setModel(treeModelForOneEssence);
	treeVievOneEssence->expandAll();

}

void Widget::closeEditWindow()
{
//	qDebug() << "closeEditWindow()" <<endl;
	this->setEnabled(true);
}

