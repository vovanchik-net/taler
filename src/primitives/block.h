// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Copyright (c) 2019-2021 Uladzimir (https://t.me/vovanchik_net)
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include <primitives/transaction.h>
#include <serialize.h>
#include <uint256.h>

enum
{
    BLOCK_PROOF_OF_STAKE = (1 << 0), // is proof-of-stake block
    BLOCK_STAKE_ENTROPY  = (1 << 1), // entropy bit for stake modifier
    BLOCK_STAKE_MODIFIER = (1 << 2), // regenerated stake modifier
    BLOCK_NEW_FORMAT = (1 << 31), // postfork block format
};

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */
class CBlockHeader
{
public:
    // header
    static const int32_t NORMAL_SERIALIZE_SIZE=84;
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    //pos
    mutable uint32_t nFlags;

    CBlockHeader()
    {
        SetNull();
    }

    bool IsNewestFormat() const {
        return (nVersion & 0xFF010000) == 0x00010000;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
        if (IsNewestFormat()) {
            if (ser_action.ForRead()) nFlags = 0;
        } else if (!(s.GetVersion() & SERIALIZE_BLOCK_LEGACY)) {
            READWRITE(nFlags);
        } else if ((s.GetType() & SER_GETHASH) && (nFlags & BLOCK_PROOF_OF_STAKE)) {
            uint32_t flags = nFlags & BLOCK_PROOF_OF_STAKE;
            READWRITE(flags);
        } else if (ser_action.ForRead()) {
            nFlags = 0;
        }
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        nFlags = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 GetPoWHash(int nHeight, const Consensus::Params& params) const;

    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    bool IsProofOfStake() const
    {
        return IsNewestFormat() ? (nVersion & 0x00020000) : (nFlags & BLOCK_PROOF_OF_STAKE);
    }

    void SetProofOfStake() {
        if (IsNewestFormat()) { nVersion = (nVersion & 0x00FFFFFF) | 0x00020000; nFlags = 0; } else { nFlags |= BLOCK_PROOF_OF_STAKE; }
    }

    bool IsNewFormatBlock() const {
        return nFlags & BLOCK_NEW_FORMAT;
    }

    void SetNewFormatBlock() const {
        nFlags |= BLOCK_NEW_FORMAT;
    }
    
    int GetVersion() const {
        return nVersion & 0xFFFF;
    }
    
    void SetVersion(int ver) {
        nVersion = (nVersion & 0x00FF0000) | 0x00010000 | (ver & 0x0000FFFF); 
        nFlags = 0;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransactionRef> vtx;

    // pos: block signature - signed by coin base txout[0]'s owner
    std::vector<unsigned char> vchBlockSig;

    // memory only
    mutable bool fChecked;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *(static_cast<CBlockHeader*>(this)) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITEAS(CBlockHeader, *this);
        READWRITE(vtx);
        if (s.GetVersion() & SERIALIZE_BLOCK_LEGACY) {
            // nothing
        } else if (IsProofOfStake() && !IsNewestFormat()) {
            READWRITE(vchBlockSig);
        } else if (ser_action.ForRead()) {
            vchBlockSig.clear();
        }
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        fChecked = false;
        vchBlockSig.clear();
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        block.nFlags         = nFlags;
        return block;
    }

    bool IsProofOfWork() const
    {
        return !IsProofOfStake();
    }

    std::string ToString() const;
};

/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    explicit CBlockLocator(const std::vector<uint256>& vHaveIn) : vHave(vHaveIn) {}

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        int nVersion = s.GetVersion();
        if (!(s.GetType() & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull() const
    {
        return vHave.empty();
    }
};

#endif // BITCOIN_PRIMITIVES_BLOCK_H
