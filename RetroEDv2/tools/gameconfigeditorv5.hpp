#ifndef GAMECONFIGEDITOR_V5_H
#define GAMECONFIGEDITOR_V5_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui
{
class GameConfigEditorv5;
}
QT_END_NAMESPACE

class GameConfigEditorv5 : public QWidget
{
    Q_OBJECT

public:
    class ActionState
    {
    public:
        QString name = "Action";

        RSDKv5::GameConfig gameConfig;
        RSDKv5::RSDKConfig rsdkconfig;

        RSDKv5::StageConfig stageconfig;
    };

    GameConfigEditorv5(QString configPath, byte type, bool oldVer, QWidget *parent = nullptr);
    ~GameConfigEditorv5();

    void setupUI(bool allowRowChange = true);

    inline void UpdateTitle(bool modified)
    {
        this->modified = modified;
        if (modified)
            emit TitleChanged(tabTitle + " *");
        else
            emit TitleChanged(tabTitle);
    }

signals:
    void TitleChanged(QString title);

protected:
    bool event(QEvent *event);

private:
    void UndoAction();
    void RedoAction();
    void ResetAction();
    void DoAction(QString name = "Action", bool setModified = true);
    void ClearActions();

    void copyConfig(ActionState *stateDst, ActionState *stateSr);

    RSDKv5::GameConfig gameConfig;
    RSDKv5::RSDKConfig rsdkConfig;

    RSDKv5::StageConfig stageConfig;

    bool oldVer = false;

    QStandardItemModel *sceneModel = nullptr;

    QList<ActionState> actions;
    int actionIndex = 0;

    bool modified    = false;
    QString tabTitle = "GameConfig Editor";

private:
    Ui::GameConfigEditorv5 *ui;
};
#endif // GAMECONFIGEDITOR_V5_H
