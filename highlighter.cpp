#include "highlighter.h"

#include <QtGui>

Highlighter::Highlighter(QObject *parent) :
    QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\balignas\\b"
                    << "\\balignof\\b"
                    << "\\band\\b"
                    << "\\band_eq\\b"
                    << "\\basm\\b"
                    << "\\bauto\\b"
                    << "\\bbitand\\b"
                    << "\\bbitor\\b"
                    << "\\bbool\\b"
                    << "\\bbreak\\b"
                    << "\\bcase\\b"
                    << "\\bcatch\\b"
                    << "\\bchar\\b"
                    << "\\bchar16_t\\b"
                    << "\\bchar32_t\\b"
                    << "\\bclass\\b"
                    << "\\bcompl\\b"
                    << "\\bconst\\b"
                    << "\\bconstexpr\\b"
                    << "\\bconst_cast\\b"
                    << "\\bcontinue\\b"
                    << "\\bdecltype\\b"
                    << "\\bdefault\\b"
                    << "\\bdelete\\b"
                    << "\\bdo\\b"
                    << "\\bdouble\\b"
                    << "\\bdynamic_cast\\b"
                    << "\\belse\\b"
                    << "\\benum\\b"
                    << "\\bexplicit\\b"
                    << "\\bexport\\b"
                    << "\\bextern\\b"
                    << "\\bfalse\\b"
                    << "\\bfloat\\b"
                    << "\\bfor\\b"
                    << "\\bfriend\\b"
                    << "\\bgoto\\b"
                    << "\\bif\\b"
                    << "\\binline\\b"
                    << "\\bint\\b"
                    << "\\blong\\b"
                    << "\\bmutable\\b"
                    << "\\bnamespace\\b"
                    << "\\bnew\\b"
                    << "\\bnoexcept\\b"
                    << "\\bnot\\b"
                    << "\\bnot_eq\\b"
                    << "\\bnullptr\\b"
                    << "\\boperator\\b"
                    << "\\bor\\b"
                    << "\\bor_eq\\b"
                    << "\\bprivate\\b"
                    << "\\bprotected\\b"
                    << "\\bpublic\\b"
                    << "\\bregister\\b"
                    << "\\breinterpret_cast\\b"
                    << "\\breturn\\b"
                    << "\\bshort\\b"
                    << "\\bsigned\\b"
                    << "\\bsizeof\\b"
                    << "\\bstatic\\b"
                    << "\\bstatic_assert\\b"
                    << "\\bstatic_cast\\b"
                    << "\\bstring\\b"
                    << "\\bstruct\\b"
                    << "\\bswitch\\b"
                    << "\\btemplate\\b"
                    << "\\bthis\\b"
                    << "\\bthread_local\\b"
                    << "\\bthrow\\b"
                    << "\\btrue\\b"
                    << "\\btry\\b"
                    << "\\btypedef\\b"
                    << "\\btypeid\\b"
                    << "\\btypename\\b"
                    << "\\bunion\\b"
                    << "\\bunsigned\\b"
                    << "\\busing\\b"
                    << "\\bvirtual\\b"
                    << "\\bvoid\\b"
                    << "\\bvolatile\\b"
                    << "\\bwchar_t\\b"
                    << "\\bwhile\\b"
                    << "\\bxor\\b"
                    << "\\bxor_eq\\b"
                    << "\\buchar\\b"
                    << "\\bushort\\b"
                    << "\\buint\\b";

    // ключевые выражения
    foreach (QString pattern, keywordPatterns) {
        rule.pattern = QRegExp(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    // #define
    rule.pattern = QRegExp("#[A-Za-z]+");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    // QApplication::
    rule.pattern = QRegExp("[a-zA-Z0-9_]+(?=\\::)");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    // указатели
    keywordFormat.setForeground(Qt::darkYellow);
    keywordFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegExp("\\b[a-zA-Z0-9_]+(?=\\->)");
    rule.format = keywordFormat;
    highlightingRules.append(rule);
    rule.pattern = QRegExp("\\b[a-zA-Z0-9_]+(?=\\.)");
    highlightingRules.append(rule);

    // КОНСТАНТЫ
    keywordFormat.setForeground(Qt::black);
    rule.pattern = QRegExp("\\b[A-Z0-9_]+\\b");
    rule.format = keywordFormat;
    highlightingRules.append(rule);

    // цифры
    classFormat.setForeground(Qt::darkBlue);
    rule.pattern = QRegExp("\\b[0-9]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    // функции
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    // // комментарий
    singleLineCommentFormat.setForeground(Qt::red);
    rule.format = singleLineCommentFormat;
    rule.pattern = QRegExp("//[^\n]*");
    highlightingRules.append(rule);

    // комментарии
    multiLineCommentFormat.setForeground(Qt::red);

    // комментарий
    commentStartExpression = QRegExp("/\\*");
    commentEndExpression = QRegExp("\\*/");

    // "текст"
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegExp("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    // <C://file.h>
    rule.pattern = QRegExp("<[A-Za-z0-9_/\\:. ]+>");
    highlightingRules.append(rule);

    // 'текст'
    rule.pattern = QRegExp("\'.*\'");
    highlightingRules.append(rule);
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (HighlightingRule rule, highlightingRules) {
        QRegExp expression(rule.pattern);
        int index = text.indexOf(expression);
        while (index >= 0) {
            int length = expression.matchedLength();
            setFormat(index, length, rule.format);
            index = text.indexOf(expression, index + length);
        }
    }
    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);

    while (startIndex >= 0) {
        int endIndex = text.indexOf(commentEndExpression, startIndex);
        int commentLength;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                    + commentEndExpression.matchedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression,
                                  startIndex + commentLength);
    }
}
