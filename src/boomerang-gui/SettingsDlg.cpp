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


#include "boomerang/core/Boomerang.h"
#include "boomerang/core/Project.h"
#include "boomerang/util/Log.h"
#include "boomerang/type/TypeRecovery.h"
#include "boomerang-gui/Decompiler.h"
#include "boomerang-gui/ui_SettingsDlg.h"

Q_DECLARE_METATYPE(ITypeRecovery *)


SettingsDlg::SettingsDlg(Decompiler *decompiler, QWidget *_parent)
    : QDialog(_parent)
    , ui(new Ui::SettingsDlg)
{
    ui->setupUi(this);

    // fill combo box with possible logging levels
    ui->cbLogLevel->clear();
    ui->cbLogLevel->setEditable(false);
    ui->cbLogLevel->addItem("Fatal",     static_cast<int>(LogLevel::Fatal));
    ui->cbLogLevel->addItem("Error",     static_cast<int>(LogLevel::Error));
    ui->cbLogLevel->addItem("Warning",   static_cast<int>(LogLevel::Warning));
    ui->cbLogLevel->addItem("Message",   static_cast<int>(LogLevel::Message));
    ui->cbLogLevel->addItem("Verbose 1", static_cast<int>(LogLevel::Verbose1));
    ui->cbLogLevel->addItem("Verbose 2", static_cast<int>(LogLevel::Verbose2));

    for (int i = 0; i < ui->cbLogLevel->count(); i++) {
        LogLevel cmbLevel = static_cast<LogLevel>(ui->cbLogLevel->itemData(i).value<int>());
        if (cmbLevel == Log::getOrCreateLog().getLogLevel()) {
            ui->cbLogLevel->setCurrentIndex(i);
            break;
        }
    }

    ui->chkDebugDecoder->setChecked(SETTING(debugDecoder));
    ui->chkDebugGenerator->setChecked(SETTING(debugGen));
    ui->chkDebugLiveness->setChecked(SETTING(debugLiveness));
    ui->chkDebugProof->setChecked(SETTING(debugProof));
    ui->chkDebugSwitch->setChecked(SETTING(debugSwitch));
    ui->chkDebugTA->setChecked(SETTING(debugTA));
    ui->chkDebugUnused->setChecked(SETTING(debugUnused));
    ui->chkTraceDecoder->setChecked(SETTING(traceDecoder));
    ui->chkVerbose->setChecked(SETTING(verboseOutput));

    // Decode settings
    ui->chkDecodeChildren->setChecked(SETTING(decodeChildren));
    ui->chkDecodeMain->setChecked(SETTING(decodeMain));
    ui->chkDecodeThruIndirectCall->setChecked(SETTING(decodeThruIndCall));
    ui->chkGenCallGraph->setChecked(SETTING(generateCallGraph));

    // Decompile settings
    ui->cbDotFile->addItem(SETTING(dotFile));
    ui->cbDotFile->setEditable(false);
    ui->spbPropMaxDepth->setRange(0, std::numeric_limits<int>::max());
    ui->spbPropMaxDepth->setValue(SETTING(propMaxDepth));
    ui->spbNumToPropagate->setRange(-1, std::numeric_limits<int>::max());
    ui->spbNumToPropagate->setValue(SETTING(numToPropagate));

    ITypeRecovery *rec = decompiler->getProject()->getTypeRecoveryEngine();
    ui->cbTypeRecoveryEngine->addItem("<None>",       QVariant::fromValue<ITypeRecovery *>(nullptr));
    ui->cbTypeRecoveryEngine->addItem(rec->getName(), QVariant::fromValue<ITypeRecovery *>(rec));
    ui->cbTypeRecoveryEngine->setCurrentIndex(SETTING(dfaTypeAnalysis) ? 1 : 0);

    ui->chkAssumeABI->setChecked(SETTING(assumeABI));
    ui->chkBranchSimplify->setChecked(SETTING(branchSimplify));
    ui->chkChangeSignatures->setChecked(SETTING(changeSignatures));
    ui->chkDecompile->setChecked(SETTING(decompile));
    ui->chkExperimental->setChecked(SETTING(experimental));
    ui->chkGenSymbols->setChecked(SETTING(generateSymbols));
    ui->chkNameParameters->setChecked(SETTING(nameParameters));
    ui->chkPropOnlyToAll->setChecked(SETTING(propOnlyToAll));
    ui->chkRemoveLabels->setChecked(SETTING(removeLabels));
    ui->chkRemoveNull->setChecked(SETTING(removeNull));
    ui->chkRemoveReturns->setChecked(SETTING(removeReturns));
    ui->chkStopAtDebugPoints->setChecked(SETTING(stopAtDebugPoints));
    ui->chkUseDataflow->setChecked(SETTING(useDataflow));
    ui->chkUseGlobals->setChecked(SETTING(useGlobals));
    ui->chkUseLocals->setChecked(SETTING(useLocals));
    ui->chkUsePromotion->setChecked(SETTING(usePromotion));
    ui->chkUseProof->setChecked(SETTING(useProof));
}


SettingsDlg::~SettingsDlg()
{
    delete ui;
}


void SettingsDlg::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);

    switch (e->type())
    {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;

    default:
        break;
    }
}


void SettingsDlg::on_btnApply_clicked()
{
    Log::getOrCreateLog().setLogLevel(static_cast<LogLevel>(ui->cbLogLevel->currentData().value<int>()));

    SETTING(debugDecoder)  = ui->chkDebugDecoder->isChecked();
    SETTING(debugGen)      = ui->chkDebugGenerator->isChecked();
    SETTING(debugLiveness) = ui->chkDebugLiveness->isChecked();
    SETTING(debugProof)    = ui->chkDebugProof->isChecked();
    SETTING(debugSwitch)   = ui->chkDebugSwitch->isChecked();
    SETTING(debugTA)       = ui->chkDebugTA->isChecked();
    SETTING(debugUnused)   = ui->chkDebugUnused->isChecked();
    SETTING(traceDecoder)  = ui->chkTraceDecoder->isChecked();
    SETTING(verboseOutput) = ui->chkVerbose->isChecked();

    // Decode
    SETTING(decodeChildren)    = ui->chkDecodeChildren->isChecked();
    SETTING(decodeMain)        = ui->chkDecodeMain->isChecked();
    SETTING(decodeThruIndCall) = ui->chkDecodeThruIndirectCall->isChecked();
    SETTING(generateCallGraph) = ui->chkGenCallGraph->isChecked();

    // Decompile
    SETTING(dotFile)         = ui->cbDotFile->currentData().value<QString>();
    SETTING(propMaxDepth)    = ui->spbPropMaxDepth->value();
    SETTING(numToPropagate)  = ui->spbNumToPropagate->value();
    SETTING(dfaTypeAnalysis) = ui->cbTypeRecoveryEngine->currentData().value<ITypeRecovery *>() != nullptr;

    SETTING(assumeABI)           = ui->chkAssumeABI->isChecked();
    SETTING(branchSimplify)      = ui->chkBranchSimplify->isChecked();
    SETTING(changeSignatures)    = ui->chkChangeSignatures->isChecked();
    SETTING(decompile)           = ui->chkDecompile->isChecked();
    SETTING(experimental)        = ui->chkExperimental->isChecked();
    SETTING(generateSymbols)     = ui->chkGenSymbols->isChecked();
    SETTING(nameParameters)      = ui->chkNameParameters->isChecked();
    SETTING(propOnlyToAll)       = ui->chkPropOnlyToAll->isChecked();
    SETTING(removeLabels)        = ui->chkRemoveLabels->isChecked();
    SETTING(removeNull)          = ui->chkRemoveNull->isChecked();
    SETTING(removeReturns)       = ui->chkRemoveReturns->isChecked();
    SETTING(stopAtDebugPoints)   = ui->chkStopAtDebugPoints->isChecked();
    SETTING(useDataflow)         = ui->chkUseDataflow->isChecked();
    SETTING(useGlobals)          = ui->chkUseGlobals->isChecked();
    SETTING(useLocals)           = ui->chkUseLocals->isChecked();
    SETTING(usePromotion)        = ui->chkUsePromotion->isChecked();
    SETTING(useProof)            = ui->chkUseProof->isChecked();
}


void SettingsDlg::on_btnOk_clicked()
{
    on_btnApply_clicked();
    accept();
}
