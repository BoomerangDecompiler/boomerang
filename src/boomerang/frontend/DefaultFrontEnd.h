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

#include <memory>


class Function;
class UserProc;
class RTL;
class IDecoder;
class Exp;
class Prog;
struct DecodeResult;
class Signature;
class Statement;
class CallStatement;
class BinaryFile;
class ISymbolProvider;

class QString;


using SharedExp      = std::shared_ptr<Exp>;
using SharedConstExp = std::shared_ptr<const Exp>;
using RTLList        = std::list<std::unique_ptr<RTL>>;


/**
 * Contains the default implementation of the source
 * indendent parts of the front end:
 * Decoding machine instructions into a control flow graph
 * populated with low and high level RTLs.
 */
class DefaultFrontEnd
{
public:
    /**
     * \param prog   program being decoded
     */
    DefaultFrontEnd(BinaryFile *binaryFile, Prog *prog);
    DefaultFrontEnd(const DefaultFrontEnd&) = delete;
    DefaultFrontEnd(DefaultFrontEnd&&) = default;

    virtual ~DefaultFrontEnd();

    DefaultFrontEnd& operator=(const DefaultFrontEnd&) = delete;
    DefaultFrontEnd& operator=(DefaultFrontEnd&&) = default;

public:
    /// Is this a win32 frontend?
    /// \note Returns false if no binary is loaded.
    bool isWin32() const;

    /**
     * Determines whether the proc with name \p procName returns or not (like abort)
     */
    bool isNoReturnCallDest(const QString& procName) const;

    /// \returns an enum identifer for this frontend's platform
    virtual Platform getType() const = 0;

    /// Accessor function to get the decoder.
    IDecoder *getDecoder() { return m_decoder.get(); }

    /// returns a symbolic name for a register index
    QString getRegName(int idx) const;
    int getRegSize(int idx);

    bool addSymbolsFromSymbolFile(const QString& fname);

    /// lookup a library signature by name
    std::shared_ptr<Signature> getLibSignature(const QString& name);

    /// return a signature that matches the architecture best
    std::shared_ptr<Signature> getDefaultSignature(const QString& name);

    virtual std::vector<SharedExp>& getDefaultParams() = 0;

    virtual std::vector<SharedExp>& getDefaultReturns() = 0;


    /// Add a "hint" that an instruction at the given address references a named global
    void addRefHint(Address addr, const QString& name) { m_refHints[addr] = name; }

    virtual bool decodeInstruction(Address pc, DecodeResult& result);

    /// Do extra processing of call instructions.
    virtual void extraProcessCall(CallStatement * /*call*/, const RTLList& /*BB_rtls*/) {}

    void readLibraryCatalog();                        ///< read from default catalog

    /// Decode all undecoded procedures and return a new program containing them.
    bool decodeEntryPointsRecursive(bool decodeMain = true);

    /// Decode all procs starting at a given address
    bool decodeRecursive(Address addr);

    /// Decode all undecoded functions.
    bool decodeUndecoded();

    /// Decode one proc starting at a given address
    /// \p addr should be the address of an UserProc
    bool decodeOnly(Address addr);

    /// Decode a fragment of a procedure, e.g. for each destination of a switch statement
    bool decodeFragment(UserProc *proc, Address addr);


    /**
     * Process a procedure, given a native (source machine) address.
     * This is the main function for decoding a procedure. It is usually overridden in the derived
     * class to do source machine specific things. If \p frag is set, we are decoding just a fragment of the proc
     * (e.g. each arm of a switch statement is decoded). If \p spec is set, this is a speculative decode.
     *
     * \param addr the address at which the procedure starts
     * \param proc the procedure object
     * \param os   the output stream for .rtl output
     * \param frag if true, this is just a fragment of a procedure
     * \param spec if true, this is a speculative decode
     *
     * \note This is a sort of generic front end. For many processors, this will be overridden
     *  in the FrontEnd derived class, sometimes calling this function to do most of the work.
     *
     * \returns true for a good decode (no illegal instructions)
     */
    virtual bool processProc(Address addr, UserProc *proc, QTextStream& os, bool frag = false, bool spec = false);

    /**
     * Given the dest of a call, determine if this is a machine specific helper function with special semantics.
     * If so, return true and set the semantics in lrtl.
     *
     * param addr the native address of the call instruction
     */
    virtual bool isHelperFunc(Address /*dest*/, Address /*addr*/, RTLList& /*lrtl*/) { return false; }

    /// Locate the entry address of "main", returning a native address
    virtual Address getMainEntryPoint(bool& gotMain) = 0;

    /**
     * Returns a list of all available entrypoints.
     */
    std::vector<Address> getEntryPoints();

    /**
     * Create a Return or a Oneway BB if a return statement already exists.
     * \param proc      pointer to enclosing UserProc
     * \param BB_rtls   list of RTLs for the current BB (not including \p returnRTL)
     * \param returnRTL pointer to the current RTL with the semantics for the return statement
     *                  (including a ReturnStatement as the last statement)
     * \returns  Pointer to the newly created BB
     */
    BasicBlock *createReturnBlock(UserProc *proc, std::unique_ptr<RTLList> BB_rtls, std::unique_ptr<RTL> returnRTL);

    /**
     * Add a synthetic return instruction and basic block (or a branch to the existing return instruction).
     *
     * \note the call BB should be created with one out edge (the return or branch BB)
     * \param callBB  the call BB that will be followed by the return or jump
     * \param proc    the enclosing UserProc
     * \param callRTL the current RTL with the call instruction
     */
    void appendSyntheticReturn(BasicBlock *callBB, UserProc *proc, RTL *callRTL);

    /**
     * Add an RTL to the map from native address to previously-decoded-RTLs. Used to restore case statements and
     * decoded indirect call statements in a new decode following analysis of such instructions. The CFG is
     * incomplete in these cases, and needs to be restarted from scratch
     */
    void saveDecodedRTL(Address a, RTL *rtl) { m_previouslyDecoded[a] = rtl; }
    void preprocessProcGoto(std::list<Statement *>::iterator ss, Address dest, const std::list<Statement *>& sl, RTL *originalRTL);
    void checkEntryPoint(std::vector<Address>& entrypoints, Address addr, const char *type);

private:
    bool refersToImportedFunction(const SharedExp& exp);

protected:
    std::unique_ptr<ISymbolProvider> m_symbolProvider;
    std::unique_ptr<IDecoder> m_decoder;
    BinaryFile *m_binaryFile;
    Prog *m_program;

    TargetQueue m_targetQueue; ///< Holds the addresses that still need to be processed

    /// Map from address to meaningful name
    std::map<Address, QString> m_refHints;

    /// Map from address to previously decoded RTLs for decoded indirect control transfer instructions
    std::map<Address, RTL *> m_previouslyDecoded;
};
