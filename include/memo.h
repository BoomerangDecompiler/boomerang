/*
 * Copyright (C) 2004, Trent Waddington
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 *
 */

/*=============================================================================
 * \file        memo.h
 * OVERVIEW:    declaration of the memo class.
 *============================================================================*/

#ifndef MEMO_H
#define MEMO_H

class Memo {
  public:
    Memo(int m) : mId(m) {}
    int mId;
    virtual void doNothing() {}
    virtual ~Memo() {} // Kill gcc warning
};

class Memoisable {
  public:
    Memoisable() { cur_memo = memos.begin(); }
    virtual ~Memoisable() {}

    void takeMemo(int mId);
    void restoreMemo(int mId, bool dec = false);

    virtual Memo *makeMemo(int mId) = 0;
    virtual void readMemo(Memo *m, bool dec) = 0;

    void takeMemo();
    bool canRestore(bool dec = false);
    void restoreMemo(bool dec = false);

  protected:
    std::list<Memo *> memos;
    std::list<Memo *>::iterator cur_memo;
};

#endif
