#include "scieditorplugin.h"
#include "scieditorwidget.h"

LMSciEditorPlugin::LMSciEditorPlugin(QObject *parent):QObject(parent)
{
}

LMSciEditorPlugin::~LMSciEditorPlugin()
{
}


QWidget* LMSciEditorPlugin::create(const QString &act)
{
    int type = 0;

   qDebug()<<"act="<<act;
   if(act.contains("Python"))
   {
       //QSci
       type = SciEditorWidget::Python_Type;

   }
   else if(act.contains("Cpp") || act.contains("C++"))
   {
       qDebug()<<"type is cpp";
       type = SciEditorWidget::Cpp_Type;
   }
   else if(act.contains("Xml"))
   {
       type = SciEditorWidget::Xml_Type;
   }
   else if(act.contains("Json"))
   {
       type = SciEditorWidget::Json_Type;
   }
   else if(act.contains("Markdown"))
   {
       type = SciEditorWidget::Markdown_Type;
   }

   qDebug()<<"type = "<<type;

   SciEditorWidget *widget = new SciEditorWidget(nullptr, type);
   return widget;

}
