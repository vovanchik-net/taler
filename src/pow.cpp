// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Copyright (c) 2020 Uladzimir(https://t.me/vovanchik_net) for Taler
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <primitives/block.h>
#include <uint256.h>

//pos: find last block index up to pindex
const CBlockIndex *GetLastBlockIndex(const CBlockIndex *pindex, const Consensus::Params &params, bool fProofOfStake) {
    const int32_t TLRHeight = params.TLRHeight;

    if (fProofOfStake) {
        while (pindex && pindex->pprev && !pindex->IsProofOfStake()) {
            if (pindex->nHeight <= TLRHeight)
                return nullptr;
            pindex = pindex->pprev;
        }
    } else {
        while (pindex && pindex->pprev && pindex->IsProofOfStake())
            pindex = pindex->pprev;
    }

    return pindex;
}

uint32_t
GetNextWorkRequiredForPos(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params) {
    assert((pindexLast->nHeight + 1) > params.TLRHeight + params.TLRInitLim);

    const CBlockIndex *pindexPrev = GetLastBlockIndex(pindexLast, params, true);
    if (pindexPrev == nullptr)
        return UintToArith256(params.posLimit).GetCompact();

    const CBlockIndex *pindexPrevPrev = GetLastBlockIndex(pindexPrev->pprev, params, true);
    if (pindexPrevPrev == nullptr || pindexPrevPrev->pprev == nullptr)
        return UintToArith256(params.posLimit).GetCompact();

    const int64_t nActualSpacing = pindexPrev->GetBlockTime() - pindexPrevPrev->GetBlockTime();
    const int64_t nInterval = params.DifficultyAdjustmentIntervalPos();
    arith_uint256 nProofOfWorkLimit = UintToArith256(params.powLimit);
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexPrev->nBits);
    bnNew *= ((nInterval - 1) * params.nPosTargetSpacing + nActualSpacing + nActualSpacing);
    //overflow fix
    if((pindexLast->nHeight + 1)>=730000)
    {
        arith_uint256 powLimit = UintToArith256(uint256S("dfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
        if (bnNew > powLimit)
            bnNew = powLimit;
    }
    //    
    bnNew /= ((nInterval + 1) * params.nPosTargetSpacing);

    if (bnNew > nProofOfWorkLimit)
        bnNew = nProofOfWorkLimit;

    return bnNew.GetCompact();
}

unsigned int
CalculateNextWorkRequired(const CBlockIndex *pindexLast, int64_t nFirstBlockTime, const Consensus::Params &params) {
    if (params.fPowNoRetargeting)
        return pindexLast->nBits;

    // Limit adjustment step
    int64_t nActualTimespan = pindexLast->GetBlockTime() - nFirstBlockTime;
    if (nActualTimespan < params.nPowTargetTimespan / 4)
        nActualTimespan = params.nPowTargetTimespan / 4;
    if (nActualTimespan > params.nPowTargetTimespan * 4)
        nActualTimespan = params.nPowTargetTimespan * 4;

    // Retarget
    arith_uint256 bnNew;
    bnNew.SetCompact(pindexLast->nBits);
    // Taler: intermediate uint256 can overflow by 1 bit
    const arith_uint256 bnPowLimit = UintToArith256(params.powLimitLegacy);
    bool fShift = bnNew.bits() > bnPowLimit.bits() - 1;
    if (fShift)
        bnNew >>= 1;
    bnNew *= nActualTimespan;
    bnNew /= params.nPowTargetTimespan;
    if (fShift)
        bnNew <<= 1;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    return bnNew.GetCompact();
}

unsigned int
GetNextWorkRequiredBTC(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params) {
    assert(pindexLast != nullptr);
    unsigned int nProofOfWorkLimit = UintToArith256(params.powLimitLegacy).GetCompact();

    // Only change once per difficulty adjustment interval
    if ((pindexLast->nHeight + 1) % params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1) != 0) {
        if (params.fPowAllowMinDifficultyBlocks) {
            // Special difficulty rule for testnet:
            // If the new block's timestamp is more than 2* 10 minutes
            // then allow mining of a min-difficulty block.
            if (pblock->GetBlockTime() > pindexLast->GetBlockTime() +
                                         params.nPowTargetSpacing(pindexLast->nHeight) * 2)
                return nProofOfWorkLimit;
            else {
                // Return the last non-special-min-difficulty-rules-block
                const CBlockIndex *pindex = pindexLast;
                while (pindex->pprev &&
                       pindex->nHeight % params.DifficultyAdjustmentIntervalPow(pindex->nHeight) != 0 &&
                       pindex->nBits == nProofOfWorkLimit)
                    pindex = pindex->pprev;
                return pindex->nBits;
            }
        }
        return pindexLast->nBits;
    }

    // Go back by what we want to be 14 days worth of blocks
    // Taler: This fixes an issue where a 51% attack can change difficulty at will.
    // Go back the full period unless it's the first retarget after genesis. Code courtesy of Art Forz
    int64_t blockstogoback = params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1) - 1;
    if ((pindexLast->nHeight + 1) != params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1))
        blockstogoback = params.DifficultyAdjustmentIntervalPow(pindexLast->nHeight + 1);

    // Go back by what we want to be 14 days worth of blocks
    const CBlockIndex *pindexFirst = pindexLast;
    for (int i = 0; pindexFirst && i < blockstogoback; i++)
        pindexFirst = pindexFirst->pprev;

    assert(pindexFirst);

    return CalculateNextWorkRequired(pindexLast, pindexFirst->GetBlockTime(), params);
}

unsigned int DarkGravityWaveOld(const CBlockIndex *pindexLast, const Consensus::Params &params) {
    /* current difficulty formula, dash - DarkGravity v3, written by Evan Duffield - evan@dash.org */
    const CBlockIndex *BlockLastSolved = pindexLast;
    const CBlockIndex *BlockReading = pindexLast;
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = 24;
    int64_t PastBlocksMax = 24;
    int64_t CountBlocks = 0;
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;

    if (BlockLastSolved == nullptr || BlockLastSolved->nHeight == 0 || BlockLastSolved->nHeight < PastBlocksMin) {
        return UintToArith256(params.powLimit).GetCompact();
    }

    for (unsigned int i = 1; BlockReading && BlockReading->nHeight > 0; i++) {
        if (PastBlocksMax > 0 && i > PastBlocksMax) { break; }
        CountBlocks++;

        if (CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) { PastDifficultyAverage.SetCompact(BlockReading->nBits); }
            else {
                PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) +
                                         (arith_uint256().SetCompact(BlockReading->nBits))) / (CountBlocks + 1);
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if (LastBlockTime > 0) {
            int64_t Diff = (LastBlockTime - BlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = BlockReading->GetBlockTime();

        if (BlockReading->pprev == nullptr) {
            assert(BlockReading);
            break;
        }
        BlockReading = BlockReading->pprev;
    }

    arith_uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * params.nPowTargetSpacing(pindexLast->nHeight);

    if (nActualTimespan < _nTargetTimespan / 3)
        nActualTimespan = _nTargetTimespan / 3;
    if (nActualTimespan > _nTargetTimespan * 3)
        nActualTimespan = _nTargetTimespan * 3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;

    if (bnNew > UintToArith256(params.powLimit)) {
        bnNew = UintToArith256(params.powLimit);
    }

    return bnNew.GetCompact();
}

unsigned int DarkGravityWave(const CBlockIndex *pindexLast, const Consensus::Params &params) {
    /* current difficulty formula, dash - DarkGravity v3, written by Evan Duffield - evan@dash.org */
    const CBlockIndex *pBlockLastSolved = GetLastBlockIndex(pindexLast, params, false);
    int64_t nActualTimespan = 0;
    int64_t LastBlockTime = 0;
    int64_t PastBlocksMin = params.nPowAveragingWindowv2;
    int64_t nPastBlocksMax = params.nPowAveragingWindowv2;
    int64_t CountBlocks = 0;
    arith_uint256 PastDifficultyAverage;
    arith_uint256 PastDifficultyAveragePrev;

    if (pBlockLastSolved == nullptr || pBlockLastSolved->nHeight == 0 || pBlockLastSolved->nHeight < PastBlocksMin) {
        return UintToArith256(params.powLimit).GetCompact();
    }

    const CBlockIndex *pBlockReading = pBlockLastSolved;

    for (unsigned int i = 1; pBlockReading && pBlockReading->nHeight > 0; ++i) {
        if (nPastBlocksMax > 0 && i > nPastBlocksMax) {
            break;
        }

        CountBlocks++;

        if (CountBlocks <= PastBlocksMin) {
            if (CountBlocks == 1) {
                PastDifficultyAverage.SetCompact(pBlockReading->nBits);
            } else {
                PastDifficultyAverage = ((PastDifficultyAveragePrev * CountBlocks) +
                                         (arith_uint256().SetCompact(pBlockReading->nBits))) / (CountBlocks + 1);
            }
            PastDifficultyAveragePrev = PastDifficultyAverage;
        }

        if (LastBlockTime > 0) {
            int64_t Diff = (LastBlockTime - pBlockReading->GetBlockTime());
            nActualTimespan += Diff;
        }
        LastBlockTime = pBlockReading->GetBlockTime();
        
        pBlockReading = GetLastBlockIndex(pBlockReading->pprev, params, false);
    }

    arith_uint256 bnNew(PastDifficultyAverage);

    int64_t _nTargetTimespan = CountBlocks * params.nPowTargetSpacing(pindexLast->nHeight);

    if (nActualTimespan < _nTargetTimespan / 3)
        nActualTimespan = _nTargetTimespan / 3;
    if (nActualTimespan > _nTargetTimespan * 3)
        nActualTimespan = _nTargetTimespan * 3;

    // Retarget
    bnNew *= nActualTimespan;
    bnNew /= _nTargetTimespan;

    if (bnNew > UintToArith256(params.powLimit)) {
        bnNew = UintToArith256(params.powLimit);
    }

    return bnNew.GetCompact();
}

unsigned int
GetNextWorkRequiredForPow(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params) {
    int nNextHeight = pindexLast->nHeight + 1;

    if (nNextHeight < params.nLyra2ZHeight) {
        return GetNextWorkRequiredBTC(pindexLast, pblock, params);
    } else if (nNextHeight < params.nLyra2ZHeight + params.nPowAveragingWindowv1) {
        return UintToArith256(params.powLimit).GetCompact();
    } else if (nNextHeight < params.nNewDiffAdjustmentAlgorithmHeight) {
        return DarkGravityWaveOld(pindexLast, params);
    } else if (nNextHeight < params.nNewDiffAdjustmentAlgorithmHeight + params.nPowAveragingWindowv2) {
        return 0x1c08b5b1;  // 29.39
    }

    return DarkGravityWave(pindexLast, params);
}

uint32_t GetNextWork (const CBlockIndex *pLast, const Consensus::Params &params, bool fProofOfStake) {
    //v1.3 Calc difficulty PoW & PoS. Copyright by Uladzimir(https://t.me/vovanchik_net)
    arith_uint256 bnOld, bnNew, bnLimit = (~arith_uint256 (0)) >> params.newLimitShift;
    const CBlockIndex* pPrev = pLast;
    int64_t srcTimes = 0;
    int64_t destBlockCount = 24;
    int64_t LastNonBlockCount = 0;
    for (int i = 0; i < destBlockCount; i++) {
        while (pPrev && (pPrev->IsProofOfStake() != fProofOfStake)) pPrev = pPrev->pprev;
        if (pPrev == nullptr) return bnLimit.GetCompact();
        if (pPrev->pprev == nullptr) return bnLimit.GetCompact();
        if (i == 0) { bnOld.SetCompact (pPrev->nBits); LastNonBlockCount = pLast->nHeight - pPrev->nHeight; }
        if (i < 12) { bnNew = ((bnNew * i) + (arith_uint256().SetCompact(pPrev->nBits))) / (i + 1); }
        srcTimes += (pPrev->GetBlockTime() - pPrev->pprev->GetBlockTime());
        pPrev = pPrev->pprev;
    }
    bnNew /= destBlockCount * params.newTargetSpacing;
    bnNew *= srcTimes;
    while (LastNonBlockCount > 5) { bnNew *= 2; LastNonBlockCount--; if (bnNew > bnLimit) break; }
    if (bnNew > bnOld*4) bnNew = bnOld*4;
    if (bnNew < bnOld/4) bnNew = bnOld/4;
    if (bnNew > bnLimit) bnNew = bnLimit;
    return bnNew.GetCompact();
}

uint32_t GetNextWorkRequired(const CBlockIndex *pindexLast, const CBlockHeader *pblock, const Consensus::Params &params) {
    assert(pindexLast != nullptr);
    if (pindexLast->nHeight >= params.newProofHeight)
        return GetNextWork (pindexLast, params, pblock->IsProofOfStake());
    return pblock->IsProofOfStake() ?
           GetNextWorkRequiredForPos(pindexLast, pblock, params) :
           GetNextWorkRequiredForPow(pindexLast, pblock, params);
}
