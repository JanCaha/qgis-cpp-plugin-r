#include "interactiverwidget.h"

InteractiveRWidget::InteractiveRWidget( QWidget *parent ) : QgsCodeEditorR( parent, QgsCodeEditor::Mode::CommandInput )
{
    displayPrompt( false );

    InteractiveRWidget::initializeLexer();
}

void InteractiveRWidget::clear()
{
    QgsCodeEditorR::clear();
    displayPrompt( false );
}

void InteractiveRWidget::keyPressEvent( QKeyEvent *event )
{
    switch ( event->key() )
    {
        case Qt::Key_Return:
        case Qt::Key_Enter:
            emit runCommand( text() );

            break;

        default:
            QgsCodeEditorR::keyPressEvent( event );
    }
}

void InteractiveRWidget::initializeLexer()
{
    QgsCodeEditorR::initializeLexer();

    setCaretLineVisible( false );
    setLineNumbersVisible( false ); // NO linenumbers for the input line
    // Margin 1 is used for the '>' prompt (console input)
    setMarginLineNumbers( 1, true );
    setMarginWidth( 1, "00" );
    setMarginType( 1, QsciScintilla::MarginType::TextMarginRightJustified );
    setMarginsBackgroundColor( color( QgsCodeEditorColorScheme::ColorRole::Background ) );
    setEdgeMode( QsciScintilla::EdgeNone );
}

void InteractiveRWidget::displayPrompt( bool more )
{
    const QString prompt = !more ? ">" : "+";
    SendScintilla( QsciScintilla::SCI_MARGINSETTEXT, static_cast<uintptr_t>( 0 ), prompt.toUtf8().constData() );
}
