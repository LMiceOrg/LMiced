#include "scieditorwidget.h"
#include "ui_scieditorwidget.h"

#include <QFormLayout>
#include <QGridLayout>

#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QIODevice>
#include <QDebug>

SciEditorWidget::SciEditorWidget(QWidget *parent, int type) :
    QWidget(parent),
    ui(new Ui::SciEditorWidget)
{
    ui->setupUi(this);

    m_userParser = new QsciLexerHTML(this);
    m_xmlParser = new QsciLexerXML(this);
    m_cppParser = new QsciLexerCPP(this);
    m_pythonParser = new QsciLexerPython(this);
    m_jsParser = new QsciLexerJavaScript(this);

    //QFont font = m_pythonParser->font(-1);
    //font.setPixelSize(88);
    //m_pythonParser->setFont(font, -1);
    QGridLayout * layout = new QGridLayout;
    //layout->setGridGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);


    m_sci = new QsciScintilla(this);

    m_sci->setUtf8(true);
    m_sci->setText("ctx");
    m_sci->setAutoIndent(true);

    loadLexer(type);

    //m_sci->setLexer(m_pythonParser);
    m_sci->setFolding(QsciScintilla::BoxedTreeFoldStyle);
    // Show line number in left side
    m_sci->setMarginLineNumbers(QsciScintilla::SC_MARGIN_NUMBER, true);
    m_sci->setMarginWidth(QsciScintilla::SC_MARGIN_NUMBER, 32);

    m_sci->setFont(QFont("simson", 14));


    layout->addWidget(m_sci);

    this->setLayout(layout);

    //qDebug()<<"menu";
    // Install menu

    //QMenuBar * bar = new QMenuBar(0);
    //layout->addWidget(bar);

    //QMenu *menu = bar->addMenu("Editor");

    //QAction* act = menu->addAction("Open");
    QAction* act = new QAction("Open", this);
    act->setShortcut(QKeySequence::Open);
    act->setShortcutContext(Qt::WidgetShortcut);
    this->addAction(act);
    m_sci->addAction(act);
    QObject::connect(act, &QAction::triggered, [this](){
        QString name = QFileDialog::getOpenFileName(this, tr("Open"));
        if(name.isEmpty())
            return;
        QFile file(name);
        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        file.close();
        m_sci->setText(data.data());
        this->setWindowTitle(name);

    });

    // Show custom actions in popup menu
    //this->setContextMenuPolicy(Qt::ActionsContextMenu);


}

SciEditorWidget::~SciEditorWidget()
{
    delete ui;
}

void SciEditorWidget::loadLexer(int type)
{
    switch(type)
    {
    case User_Type:
        m_sci->setLexer(m_userParser);
        break;
    case Python_Type:
        m_sci->setLexer(m_pythonParser);
        break;
    case Xml_Type:
        m_sci->setLexer(m_xmlParser);
        break;
    case Json_Type:
        m_sci->setLexer(m_jsParser);
        break;
    case Cpp_Type:
        m_sci->setLexer(m_cppParser);
        break;
    case Markdown_Type:
        m_sci->setLexer(m_userParser);
        break;
    default:
        break;
    }
}
