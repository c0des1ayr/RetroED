#include "includes.hpp"
#include "ui_scenepropertiesv5.h"

ScenePropertiesv5::ScenePropertiesv5(QWidget *parent) : QWidget(parent), ui(new Ui::ScenePropertiesv5)
{
    ui->setupUi(this);

    loadGlobalCB = ui->loadGlobals;

    editTIL  = ui->editChunks;
    editSCF  = ui->editStageconfig;
    editPAL  = ui->editPalette;
    editTSet = ui->editTSet;
}

ScenePropertiesv5::~ScenePropertiesv5() { delete ui; }

void ScenePropertiesv5::setupUI(RSDKv5::Scene *scn, RSDKv5::StageConfig *config)
{
    unsetUI();

    if (!scn || !config)
        return;

    ui->loadGlobals->blockSignals(true);
    ui->loadGlobals->setChecked(config->loadGlobalObjects);
    ui->loadGlobals->blockSignals(false);
}

void ScenePropertiesv5::unsetUI() {}

#include "moc_scenepropertiesv5.cpp"