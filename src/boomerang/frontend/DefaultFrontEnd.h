#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once


#include "boomerang/frontend/SigEnum.h"
#include "boomerang/frontend/TargetQueue.h"
#include "boomerang/ifc/IFrontEnd.h"

#include <map>
#include <memory>


class Function;
class UserProc;
class RTL;
class IDecoder;
class Exp;
class Prog;
class DecodeResult;
class Signature;
class Statement;
class CallStatement;
class BinaryFile;
class MachineInstruction;

class QString;


/**
 * Contains the default implementation of the source indendent parts of the front end:
 * Decoding machine instructions into a control flow graph populated with low
 * and high level RTLs.
 */
class BOOMERANG_API DefaultFrontEnd : public IFrontEnd
{
public:
    DefaultFrontEnd(Project *project);
    DefaultFrontEnd(const DefaultFrontEnd &) = delete;
    DefaultFrontEnd(DefaultFrontEnd &&)      = default;

    virtual ~DefaultFrontEnd();

    DefaultFrontEnd &operator=(const DefaultFrontEnd &) = delete;
    DefaultFrontEnd &operator=(DefaultFrontEnd &&) = default;

public:
    bool initialize(Project *project) override;

    /// \copydoc IFrontEnd::getDecoder
    IDecoder *getDecoder() override { return m_decoder; }
    const IDecoder *getDecoder() const override { return m_decoder; }

    /// \copydoc IFrontEnd::decodeEntryPointsRecursive
    bool decodeEntryPointsRecursive(bool decodeMain = true) override;

    /// \copydoc IFrontEnd::decodeRecursive
    bool decodeRecursive(Address addr) override;

    /// \copydoc IFrontEnd::decodeUndecoded
    bool decodeUndecoded() override;

    /// \copydoc IFrontEnd::decodeFragment
    bool decodeFragment(UserProc *proc, Address addr) override;

    /// \copydoc IFrontEnd::processProc
    bool processProc(UserProc *proc, Address addr) override;

    /// Do extra processing of call instructions.
    /// Does nothing by default.
    virtual void extraProcessCall(const std::shared_ptr<CallStatement> &call,
                                  const RTLList &BB_rtls);

    /// Disassemble and lift a single instruction at address \p addr
    /// \returns true on success
    [[nodiscard]] bool decodeInstruction(Address pc, MachineInstruction &insn,
                                         DecodeResult &lifted);

public:
    /// \copydoc IFrontEnd::getEntryPoints
    std::vector<Address> findEntryPoints() override;

    /// \copydoc IFrontEnd::isNoReturnCallDest
    bool isNoReturnCallDest(const QString &procName) const override;

    /// \copydoc IFrontEnd::addRefHint
    void addRefHint(Address addr, const QString &name) override;

    /// \copydoc IFrontEnd::saveDecodedRTL
    void saveDecodedRTL(Address a, RTL *rtl) override;

protected:
    /**
     * Create a Return or a Oneway BB if a return statement already exists.
     * \param proc      pointer to enclosing UserProc
     * \param BB_rtls   list of RTLs for the current BB (not including \p returnRTL)
     * \param returnRTL pointer to the current RTL with the semantics for the return statement
     *                  (including a ReturnStatement as the last statement)
     * \returns  Pointer to the newly created BB
     */
    BasicBlock *createReturnBlock(UserProc *proc, std::unique_ptr<RTLList> bb_rtls,
                                  std::unique_ptr<RTL> returnRTL);

    /**
     * Given the dest of a call, determine if this is a machine specific helper function with
     * special semantics. If so, return true and set the semantics in \p lrtl.
     *
     * \param dest the destination of the call instruction
     * \param addr the native address of the call instruction
     * \param lrtl semnatics of the helper function, if applicable.
     */
    virtual bool isHelperFunc(Address dest, Address addr, RTLList &lrtl);

private:
    /// Disassemble a single instruction at address \p pc
    /// \returns true on success
    bool disassembleInstruction(Address pc, MachineInstruction &insn);

    /// Lifts a single instruction \p insn to an RTL.
    /// \returns true on success
    bool liftInstruction(const MachineInstruction &insn, DecodeResult &lifted);

    std::unique_ptr<RTLList> liftBB(const std::list<MachineInstruction> &bbInsns);

    /// \returns true iff \p exp is a memof that references the address of an imported function.
    bool refersToImportedFunction(const SharedExp &exp);

    /**
     * Add a synthetic return instruction and basic block (or a branch to the existing return
     * instruction).
     *
     * \note the call BB should be created with one out edge (the return or branch BB)
     * \param callBB  the call BB that will be followed by the return or jump
     * \param proc    the enclosing UserProc
     * \param callRTL the current RTL with the call instruction
     */
    //     void appendSyntheticReturn(BasicBlock *callBB, UserProc *proc, RTL *callRTL);

    /**
     * Change a jump to a call if the jump destination is an impoted function.
     * \sa refersToImportedFunction
     */
    void preprocessProcGoto(RTL::StmtList::iterator ss, Address dest, const RTL::StmtList &sl,
                            RTL *originalRTL);

    /// Creates a UserProc for the entry point at address \p addr.
    /// Returns nullptr on failure.
    UserProc *createFunctionForEntryPoint(Address entryAddr, const QString &functionType);

    /**
     * Get the address of the destination of a library thunk.
     * Returns Address::INVALID if the call is not a library thunk or an error occurred.
     */
    Address getAddrOfLibraryThunk(const std::shared_ptr<CallStatement> &call, UserProc *proc);

protected:
    IDecoder *m_decoder      = nullptr;
    BinaryFile *m_binaryFile = nullptr;
    Prog *m_program          = nullptr;

    TargetQueue m_targetQueue; ///< Holds the addresses that still need to be processed

    /// Map from address to meaningful name
    std::map<Address, QString> m_refHints;

    /// Map from address to previously decoded RTLs for decoded indirect control transfer
    /// instructions
    std::map<Address, RTL *> m_previouslyDecoded;
};
