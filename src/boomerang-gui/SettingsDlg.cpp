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
#include "boomerang/util/Log.h"
#include "boomerang/type/TypeRecovery.h"
#include "boomerang-gui/Decompiler.h"
#include "boomerang-gui/ui_SettingsDlg.h"

Q_DECLARE_METATYPE(ITypeRecovery *);


SettingsDlg::SettingsDlg(Decompiler *, QWidget *_parent)
    : QDialog(_parent)
    , ui(new Ui::SettingsDlg)
{
    ui->setupUi(this);

    // fill combo box with possible logging levels
    ui->cbLogLevel->clear();
    ui->cbLogLevel->setEditable(false);
    ui->cbLogLevel->addItem("Fatal", (int)LogLevel::Fatal);
    ui->cbLogLevel->addItem("Error", (int)LogLevel::Error);
    ui->cbLogLevel->addItem("Warning", (int)LogLevel::Warning);
    ui->cbLogLevel->addItem("Message", (int)LogLevel::Message);
    ui->cbLogLevel->addItem("Verbose 1", (int)LogLevel::Verbose1);
    ui->cbLogLevel->addItem("Verbose 2", (int)LogLevel::Verbose2);

    for (int i = 0; i < ui->cbLogLevel->count(); i++) {
        LogLevel cmbLevel = (LogLevel)(ui->cbLogLevel->itemData(i).value<int>());
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
    ui->chkVerbose->setChecked(SETTING(vFlag));

    // Decode settings
    ui->chkDecodeChildren->setChecked(!SETTING(noDecodeChildren));
    ui->chkDecodeMain->setChecked(SETTING(decodeMain));
    ui->chkDecodeThruIndirectCall->setChecked(SETTING(decodeThruIndCall));
    ui->chkFastDecode->setEnabled(false);
    ui->chkGenCallGraph->setChecked(SETTING(generateCallGraph));

    // Decompile settings
    ui->chkDecompile->setChecked(!SETTING(noDecompile));
    ui->chkStopBeforeDecompile->setChecked(SETTING(stopBeforeDecompile));
    ui->cbDotFile->addItem(SETTING(dotFile));
    ui->cbDotFile->setEditable(false);
    ui->spbPropMaxDepth->setRange(0, std::numeric_limits<int>::max());
    ui->spbPropMaxDepth->setValue(SETTING(propMaxDepth));
    ui->spbNumToPropagate->setRange(-1, std::numeric_limits<int>::max());
    ui->spbNumToPropagate->setValue(SETTING(numToPropagate));

    ITypeRecovery *rec = Boomerang::get()->getOrCreateProject()->getTypeRecoveryEngine();
    ui->cbTypeRecoveryEngine->addItem("<None>",       QVariant::fromValue<ITypeRecovery *>(nullptr));
    ui->cbTypeRecoveryEngine->addItem(rec->getName(), QVariant::fromValue<ITypeRecovery *>(rec));
    ui->cbTypeRecoveryEngine->setCurrentIndex(SETTING(dfaTypeAnalysis) ? 1 : 0);

    ui->spbMaxMemDepth->setRange(0, std::numeric_limits<int>::max());
    ui->spbMaxMemDepth->setValue(SETTING(maxMemDepth));

    ui->chkAssumeABI->setChecked(SETTING(assumeABI));
    ui->chkBranchSimplify->setChecked(!SETTING(noBranchSimplify));
    ui->chkChangeSignatures->setChecked(!SETTING(noChangeSignatures));
    ui->chkDecompile->setChecked(!SETTING(noDecompile));
    ui->chkExperimental->setChecked(SETTING(experimental));
    ui->chkGenSymbols->setChecked(SETTING(generateSymbols));
    ui->chkNameParameters->setChecked(!SETTING(noParameterNames));
    ui->chkPropOnlyToAll->setChecked(SETTING(propOnlyToAll));
    ui->chkRemoveLabels->setChecked(!SETTING(noRemoveLabels));
    ui->chkRemoveNull->setChecked(!SETTING(noRemoveNull));
    ui->chkRemoveReturns->setChecked(!SETTING(noRemoveReturns));
    ui->chkStopAtDebugPoints->setChecked(SETTING(stopAtDebugPoints));
    ui->chkStopBeforeDecompile->setChecked(SETTING(stopBeforeDecompile));
    ui->chkUseDataflow->setChecked(!SETTING(noDataflow));
    ui->chkUseGlobals->setChecked(!SETTING(noGlobals));
    ui->chkUseLocals->setChecked(!SETTING(noLocals));
    ui->chkUsePromotion->setChecked(!SETTING(noPromote));
    ui->chkUseProof->setChecked(!SETTING(noProve));

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
    Log::getOrCreateLog().setLogLevel((LogLevel)ui->cbLogLevel->currentData().value<int>());

    SETTING(debugDecoder)  = ui->chkDebugDecoder->isChecked();
    SETTING(debugGen)      = ui->chkDebugGenerator->isChecked();
    SETTING(debugLiveness) = ui->chkDebugLiveness->isChecked();
    SETTING(debugProof)    = ui->chkDebugProof->isChecked();
    SETTING(debugSwitch)   = ui->chkDebugSwitch->isChecked();
    SETTING(debugTA)       = ui->chkDebugTA->isChecked();
    SETTING(debugUnused)   = ui->chkDebugUnused->isChecked();
    SETTING(traceDecoder)  = ui->chkTraceDecoder->isChecked();
    SETTING(vFlag)         = ui->chkVerbose->isChecked();

    // Decode
    SETTING(noDecodeChildren)  = !ui->chkDecodeChildren->isChecked();
    SETTING(decodeMain)        = ui->chkDecodeMain->isChecked();
    SETTING(decodeThruIndCall) = ui->chkDecodeThruIndirectCall->isChecked();
    SETTING(generateCallGraph) = ui->chkGenCallGraph->isChecked();

    // Decompile
    SETTING(dotFile)         = ui->cbDotFile->currentData().value<QString>();
    SETTING(propMaxDepth)    = ui->spbPropMaxDepth->value();
    SETTING(numToPropagate)  = ui->spbNumToPropagate->value();
    SETTING(dfaTypeAnalysis) = ui->cbTypeRecoveryEngine->currentData().value<ITypeRecovery *>() != nullptr;
    SETTING(maxMemDepth)     = ui->spbMaxMemDepth->value();

    SETTING(assumeABI)           = ui->chkAssumeABI->isChecked();
    SETTING(noBranchSimplify)    = !ui->chkBranchSimplify->isChecked();
    SETTING(noChangeSignatures)  = !ui->chkChangeSignatures->isChecked();
    SETTING(noDecompile)         = !ui->chkDecompile->isChecked();
    SETTING(experimental)        = ui->chkExperimental->isChecked();
    SETTING(generateSymbols)     = ui->chkGenSymbols->isChecked();
    SETTING(noParameterNames)    = !ui->chkNameParameters->isChecked();
    SETTING(propOnlyToAll)       = ui->chkPropOnlyToAll->isChecked();
    SETTING(noRemoveLabels)      = !ui->chkRemoveLabels->isChecked();
    SETTING(noRemoveNull)        = !ui->chkRemoveNull->isChecked();
    SETTING(noRemoveReturns)     = !ui->chkRemoveReturns->isChecked();
    SETTING(stopAtDebugPoints)   = ui->chkStopAtDebugPoints->isChecked();
    SETTING(stopBeforeDecompile) = ui->chkStopBeforeDecompile->isChecked();
    SETTING(noDataflow)          = !ui->chkUseDataflow->isChecked();
    SETTING(noGlobals)           = !ui->chkUseGlobals->isChecked();
    SETTING(noLocals)            = !ui->chkUseLocals->isChecked();
    SETTING(noPromote)           = !ui->chkUsePromotion->isChecked();
    SETTING(noProve)             = !ui->chkUseProof->isChecked();
}


void SettingsDlg::on_btnOk_clicked()
{
    on_btnApply_clicked();
    accept();
}
