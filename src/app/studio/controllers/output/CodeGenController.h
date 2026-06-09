#ifndef PIXELSTUDIO_APP_CONVERTER_CODEGENCONTROLLER_H
#define PIXELSTUDIO_APP_CONVERTER_CODEGENCONTROLLER_H

#include <QObject>

class DisplayConverter;
struct ConverterState;

class CodeGenController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString arrayName READ arrayName WRITE setArrayName NOTIFY arrayNameChanged)
    Q_PROPERTY(bool codeIncludeComments READ codeIncludeComments WRITE setCodeIncludeComments NOTIFY codeGenOptionsChanged)
    Q_PROPERTY(bool codeUseProgmem READ codeUseProgmem WRITE setCodeUseProgmem NOTIFY codeGenOptionsChanged)
    Q_PROPERTY(bool codeStaticStorage READ codeStaticStorage WRITE setCodeStaticStorage NOTIFY codeGenOptionsChanged)
    Q_PROPERTY(bool rgb565BigEndian READ rgb565BigEndian WRITE setRgb565BigEndian NOTIFY rgb565BigEndianChanged)
    Q_PROPERTY(int codeDmaAlign READ codeDmaAlign WRITE setCodeDmaAlign NOTIFY codeDmaAlignChanged)
    Q_PROPERTY(bool showFullGeneratedCode READ showFullGeneratedCode WRITE setShowFullGeneratedCode NOTIFY showFullGeneratedCodeChanged)

public:
    explicit CodeGenController(QObject *parent = nullptr);

    void attach(DisplayConverter *host, ConverterState *state);

    QString arrayName() const;
    bool codeIncludeComments() const;
    bool codeUseProgmem() const;
    bool codeStaticStorage() const;
    bool rgb565BigEndian() const;
    int codeDmaAlign() const;
    bool showFullGeneratedCode() const;

    Q_INVOKABLE void setArrayName(const QString &name);
    Q_INVOKABLE void setCodeIncludeComments(bool on);
    Q_INVOKABLE void setCodeUseProgmem(bool on);
    Q_INVOKABLE void setCodeStaticStorage(bool on);
    Q_INVOKABLE void setRgb565BigEndian(bool on);
    Q_INVOKABLE void setCodeDmaAlign(int align);
    Q_INVOKABLE void setShowFullGeneratedCode(bool on);
    Q_INVOKABLE void copyGeneratedArray();

    void notifyAllChanged();

signals:
    void arrayNameChanged();
    void codeGenOptionsChanged();
    void rgb565BigEndianChanged();
    void codeDmaAlignChanged();
    void showFullGeneratedCodeChanged();

private:
    void requestRebuild(bool immediate = false);

    DisplayConverter *m_host = nullptr;
    ConverterState *m_state = nullptr;
};

#endif // PIXELSTUDIO_APP_CONVERTER_CODEGENCONTROLLER_H
