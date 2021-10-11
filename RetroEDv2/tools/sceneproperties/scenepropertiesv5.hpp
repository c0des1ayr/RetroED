#ifndef SCENEPROPERTIES_V5_H
#define SCENEPROPERTIES_V5_H

#include <QWidget>

namespace Ui
{
class ScenePropertiesv5;
}

class ScenePropertiesv5 : public QWidget
{
    Q_OBJECT

public:
    explicit ScenePropertiesv5(QWidget *parent = nullptr);
    ~ScenePropertiesv5();

    void setupUI(RSDKv5::Scene *scn, RSDKv5::StageConfig *config);
    void unsetUI();

    QCheckBox *loadGlobalCB = nullptr;

    QPushButton *editTIL  = nullptr;
    QPushButton *editSCF  = nullptr;
    QPushButton *editPAL  = nullptr;
    QPushButton *editTSet = nullptr;

private:
    Ui::ScenePropertiesv5 *ui;
};

#endif // SCENEPROPERTIES_V5_H