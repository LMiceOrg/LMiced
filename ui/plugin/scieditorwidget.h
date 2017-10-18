#ifndef SCIEDITORWIDGET_H
#define SCIEDITORWIDGET_H

#include <QWidget>

#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexerpython.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qsciscintilla.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerjavascript.h>

namespace Ui {
class SciEditorWidget;
}

class SciEditorWidget : public QWidget
{
    Q_OBJECT

public:
    enum LexerType{User_Type, Python_Type, Xml_Type, Json_Type, Cpp_Type, Markdown_Type};

    explicit SciEditorWidget(QWidget *parent = 0, int type=0);
    ~SciEditorWidget();
    void loadLexer(int type = 0);

private:
    Ui::SciEditorWidget *ui;

    QsciLexerHTML* m_userParser;
    QsciLexerXML* m_xmlParser;
    QsciLexerCPP* m_cppParser;
    QsciLexerPython* m_pythonParser;
    QsciLexerJavaScript* m_jsParser;
    QsciScintilla* m_sci;
};

#endif // SCIEDITORWIDGET_H
