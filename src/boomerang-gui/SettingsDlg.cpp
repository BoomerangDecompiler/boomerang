#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#include "SettingsDlg.h"

#include "boomerang-gui/Decompiler.h"
#include "boomerang-gui/ui_SettingsDlg.h"

#include "boomerang/core/Project.h"
#include "boomerang/core/Settings.h"
#include "boomerang/ifc/ITypeRecovery.h"
#include "boomerang/util/log/Log.h"


Q_DECLARE_METATYPE(ITypeRecovery *)


SettingsDlg::SettingsDlg(Decompiler *decompiler, QWidget *_parent)
    : QDialog(_parent)
    , ui(new Ui::SettingsDlg)
    , m_settings(decompiler->getProject()->getSettings())
{
    ui->setupUi(this);

    // fill combo box with possible logging levels
    ui->cbLogLevel->clear();
    ui->cbLogLevel->setEditable(false);
    ui->cbLogLevel->addItem("Fatal", static_cast<int>(LogLevel::Fatal));
    ui->cbLogLevel->addItem("Error", static_cast<int>(LogLevel::Error));
    ui->cbLogLevel->addItem("Warning", static_cast<int>(LogLevel::Warning));
    ui->cbLogLevel->addItem("Message", static_cast<int>(LogLevel::Message));
    ui->cbLogLevel->addItem("Verbose 1", static_cast<int>(LogLevel::Verbose1));
    ui->cbLogLevel->addItem("Verbose 2", static_cast<int>(LogLevel::Verbose2));

    for (int i = 0; i < ui->cbLogLevel->count(); i++) {
        LogLevel cmbLevel = static_cast<LogLevel>(ui->cbLogLevel->itemData(i).value<int>());
        if (cmbLevel == Log::getOrCreateLog().getLogLevel()) {
            ui->cbLogLevel->setCurrentIndex(i);
            break;
        }
    }

    ui->chkDebugDecoder->setChecked(m_settings->debugDecoder);
    ui->chkDebugGenerator->setChecked(m_settings->debugGen);
    ui->chkDebugLiveness->setChecked(m_settings->debugLiveness);
    ui->chkDebugProof->setChecked(m_settings->debugProof);
    ui->chkDebugSwitch->setChecked(m_settings->debugSwitch);
    ui->chkDebugTA->setChecked(m_settings->debugTA);
    ui->chkDebugUnused->setChecked(m_settings->debugUnused);
    ui->chkTraceDecoder->setChecked(m_settings->traceDecoder);
    ui->chkVerbose->setChecked(m_settings->verboseOutput);

    // Decode settings
    ui->chkDecodeChildren->setChecked(m_settings->decodeChildren);
    ui->chkDecodeMain->setChecked(m_settings->decodeMain);
    ui->chkDecodeThruIndirectCall->setChecked(m_settings->decodeThruIndCall);
    ui->chkGenCallGraph->setChecked(m_settings->generateCallGraph);

    // Decompile settings
    ui->cbDotFile->addItem(m_settings->dotFile);
    ui->cbDotFile->setEditable(false);
    ui->spbPropMaxDepth->setRange(0, std::numeric_limits<int>::max());
    ui->spbPropMaxDepth->setValue(m_settings->propMaxDepth);
    ui->spbNumToPropagate->setRange(-1, std::numeric_limits<int>::max());
    ui->spbNumToPropagate->setValue(m_settings->numToPropagate);

    ITypeRecovery *rec = decompiler->getProject()->getTypeRecoveryEngine();
    ui->cbTypeRecoveryEngine->addItem("<None>", QVariant::fromValue<ITypeRecovery *>(nullptr));
    ui->cbTypeRecoveryEngine->addItem(rec->getName(), QVariant::fromValue<ITypeRecovery *>(rec));
    ui->cbTypeRecoveryEngine->setCurrentIndex(m_settings->useTypeAnalysis ? 1 : 0);

    ui->chkAssumeABI->setChecked(m_settings->assumeABI);
    ui->chkChangeSignatures->setChecked(m_settings->changeSignatures);
    ui->chkGenSymbols->setChecked(m_settings->generateSymbols);
    ui->chkNameParameters->setChecked(m_settings->nameParameters);
    ui->chkRemoveLabels->setChecked(m_settings->removeLabels);
    ui->chkRemoveNull->setChecked(m_settings->removeNull);
    ui->chkRemoveReturns->setChecked(m_settings->removeReturns);
    ui->chkUseDataflow->setChecked(m_settings->useDataflow);
    ui->chkUseGlobals->setChecked(m_settings->useGlobals);
    ui->chkUseLocals->setChecked(m_settings->useLocals);
    ui->chkUsePromotion->setChecked(m_settings->usePromotion);
    ui->chkUseProof->setChecked(m_settings->useProof);
}


SettingsDlg::~SettingsDlg()
{
    delete ui;
}


void SettingsDlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);

    switch (e->type()) {
    case QEvent::LanguageChange: ui->retranslateUi(this); break;

    default: break;
    }
}


void SettingsDlg::on_btnApply_clicked()
{
    Log::getOrCreateLog().setLogLevel(
        static_cast<LogLevel>(ui->cbLogLevel->currentData().value<int>()));

    m_settings->debugDecoder  = ui->chkDebugDecoder->isChecked();
    m_settings->debugGen      = ui->chkDebugGenerator->isChecked();
    m_settings->debugLiveness = ui->chkDebugLiveness->isChecked();
    m_settings->debugProof    = ui->chkDebugProof->isChecked();
    m_settings->debugSwitch   = ui->chkDebugSwitch->isChecked();
    m_settings->debugTA       = ui->chkDebugTA->isChecked();
    m_settings->debugUnused   = ui->chkDebugUnused->isChecked();
    m_settings->traceDecoder  = ui->chkTraceDecoder->isChecked();
    m_settings->verboseOutput = ui->chkVerbose->isChecked();

    // Decode
    m_settings->decodeChildren    = ui->chkDecodeChildren->isChecked();
    m_settings->decodeMain        = ui->chkDecodeMain->isChecked();
    m_settings->decodeThruIndCall = ui->chkDecodeThruIndirectCall->isChecked();
    m_settings->generateCallGraph = ui->chkGenCallGraph->isChecked();

    // Decompile
    m_settings->dotFile         = ui->cbDotFile->currentData().value<QString>();
    m_settings->propMaxDepth    = ui->spbPropMaxDepth->value();
    m_settings->numToPropagate  = ui->spbNumToPropagate->value();
    m_settings->useTypeAnalysis = ui->cbTypeRecoveryEngine->currentData()
                                      .value<ITypeRecovery *>() != nullptr;

    m_settings->assumeABI        = ui->chkAssumeABI->isChecked();
    m_settings->changeSignatures = ui->chkChangeSignatures->isChecked();
    m_settings->generateSymbols  = ui->chkGenSymbols->isChecked();
    m_settings->nameParameters   = ui->chkNameParameters->isChecked();
    m_settings->removeLabels     = ui->chkRemoveLabels->isChecked();
    m_settings->removeNull       = ui->chkRemoveNull->isChecked();
    m_settings->removeReturns    = ui->chkRemoveReturns->isChecked();
    m_settings->useDataflow      = ui->chkUseDataflow->isChecked();
    m_settings->useGlobals       = ui->chkUseGlobals->isChecked();
    m_settings->useLocals        = ui->chkUseLocals->isChecked();
    m_settings->usePromotion     = ui->chkUsePromotion->isChecked();
    m_settings->useProof         = ui->chkUseProof->isChecked();
}


void SettingsDlg::on_btnOk_clicked()
{
    on_btnApply_clicked();
    accept();
}
