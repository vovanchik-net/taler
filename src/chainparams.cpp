// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Copyright (c) 2020 Uladzimir(https://t.me/vovanchik_net) for Taler
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chainparams.h>
#include <consensus/merkle.h>

#include <tinyformat.h>
#include <util.h>
#include <utilstrencodings.h>
#include <uint256.h>
#include <arith_uint256.h>

#include <assert.h>

#include <chainparamsseeds.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "Taler pershaja belaruskaja kryptavaliuta by Denis L i Serge L ";
    const CScript genesisOutputScript = CScript() << ParseHex("04f360606cf909ce34d4276ce40a5dd6a844a4a72473086e0fc635f3c4195d77df513b7541dc5f6f6d01ec39e4b729893c6d42dd5e248379a32b5259f38f6bfbae") << OP_CHECKSIG; 
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

void CChainParams::UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    consensus.vDeployments[d].nStartTime = nStartTime;
    consensus.vDeployments[d].nTimeout = nTimeout;
}

/**
 * Main network
 */

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        consensus.nSubsidyHalvingInterval = 210000 * 5;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 32256;
        consensus.BIP66Height = 32256;
        consensus.CSVHeight = 32256;
        consensus.WitnessHeight = 32256;

        consensus.powLimit = uint256S("00ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimitLegacy = uint256S("00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 10 * 60;
        consensus.nPowTargetSpacingBegin = 5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 6048;
        consensus.nMinerConfirmationWindow = 8064;
        consensus.nPosTargetTimespan = 14 * 24 * 60 * 60;       // two weeks
        consensus.nPosTargetSpacing = 60 * 7 / 3;
        consensus.nStakeMinAge = 60 * 60 * 24 * 2;              // minimum age for coin age  
        consensus.nStakeMaxAge = 60 * 60 * 24 * 90;             // stake age of full weight
        consensus.nStakeModifierInterval = 6 * 60 * 60;         // time to elapse before new modifier is computed
        consensus.posLimit = uint256S("00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.nLyra2ZHeight = 10000;
        consensus.nPowAveragingWindowv1 = 24;
        consensus.TLRHeight = 130000;
        consensus.TLRInitLim = 300;
        consensus.nNewDiffAdjustmentAlgorithmHeight = 250000;
        consensus.nPowAveragingWindowv2 = 120;
        
        consensus.newProofHeight = 10000000;
        consensus.newTargetTimespan = 3 * 60 * 60;              // 3 hour
        consensus.newTargetSpacingPoW = 2.5 * 60;               // 2.5 min 
        consensus.newTargetSpacingPoS = 5.5 * 60;               // 5.5 min 
        consensus.newLimitShift = 24;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x000000000000000000000000000000000000000000000000011a9af7a7dfce42");//800000

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0xb29f4efa935b93477cbcaf250d96bfe495c37d6c10ffd87e402b31b41d3dc56e");//800000

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 32-bit integer with any alignment.
         */
        pchMessageStart[0] = 0x64;
        pchMessageStart[1] = 0xb1;
        pchMessageStart[2] = 0x73;
        pchMessageStart[3] = 0xd8;
        nDefaultPort = 23153;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock (1505338813, 725170, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        LogPrintf("found1 is %s\n", consensus.hashGenesisBlock.ToString());
        assert(consensus.hashGenesisBlock ==
               uint256S("0xc079fd1ae86223e1522928776899d46e329da7919ca1e11be23643c67dd05d5f"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x985fae483ebbef9cde04a259282cb7465d52bf56824caf1a8132395e90488b12"));
        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as a oneshot if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        vSeeds.clear();
        vSeeds.emplace_back("dnsseed.talercrypto.com");
        vSeeds.emplace_back("dnsseed.mikalair.me");
        vSeeds.emplace_back("coin.vovanchik.net");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 65);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 50);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 193);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "tlr";

        vFixedSeeds.clear();
        //vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
                {2048, uint256S("0xc4838cab89b16915d813f424198a999af82b3dce2afed5d82cab1fe9df08d701")},
                {6602, uint256S("0xf225e2f57a5e90539a4d74b3bf1ed906a8146c64addff0f5279473fb6c5e9f0e")},
                {20000, uint256S("0x82ad64f451be0375683efbdc7d94c1b970431b02a6a3e5057dd6cd0fb2022e70")},
                {70000, uint256S("0x8c25e55d05da7fd4e61383fcdd1232e8c8ddd85b220caefc10ac6c71bdf35b3e")},
                {100000, uint256S("0xa6c3e93e8ac7b4af077a78c6ce27a1b2b8b7793a7737403bcb9e6f420a928547")},
                {145000, uint256S("0x01b12183eef6102c765d1f37ea2129e91649f849fd2b18239e7d2f7276927930")},
                {500000, uint256S("0xdbd781e1a5c96e38c6f37e85ddc79f808696ff38a107334b1d2aa0d1f3c54886")},
                {728634, uint256S("0x33e82f201a0b4074af53080d26c4092e6284bdead512b8c189b9c53526078d77")},
                {734864, uint256S("0x11367f424327987636e48a1d8d81b34d92c397a48171a53ead697ac3037ec6f3")},
                {750000, uint256S("0x45e49f1d72c78eccd0ec43997bd1ecd8a151c52b794dd80b31911d0deb7e0c9b")},
            }
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats 4096 0000000000000000002e63058c023a9a1de233554f28c7b21380b6c9003f36a8
            /* nTime    */ 1582180420,
            /* nTxCount */ 1218257,
            /* dTxRate  */ 0.02736
        };

        /* disable fallback fee on mainnet */
        m_fallback_fee_enabled = false;
    }
};

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        consensus.nSubsidyHalvingInterval = 20000;
        consensus.BIP34Height = 0;
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 10;
        consensus.BIP66Height = 10;
        consensus.CSVHeight = 10;
        consensus.WitnessHeight = 10;

        consensus.powLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimitLegacy = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 10 * 60;
        consensus.nPowTargetSpacingBegin = 5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.nRuleChangeActivationThreshold = 9;
        consensus.nMinerConfirmationWindow = 10;
        consensus.nPosTargetTimespan = 14 * 24 * 60 * 60;       // two weeks
        consensus.nPosTargetSpacing = 60 * 7 / 3;
        consensus.nStakeMinAge = 60 * 60 * 3;                   // minimum age for coin age  
        consensus.nStakeMaxAge = 60 * 60 * 24 * 3;              // stake age of full weight
        consensus.nStakeModifierInterval = 6 * 60;              // time to elapse before new modifier is computed
        consensus.posLimit = uint256S("0000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");

        consensus.nLyra2ZHeight = 10;
        consensus.nPowAveragingWindowv1 = 24;
        consensus.TLRHeight = 120;
        consensus.TLRInitLim = 50;
        consensus.nNewDiffAdjustmentAlgorithmHeight = 21000;
        consensus.nPowAveragingWindowv2 = 120;

        consensus.newProofHeight = 100;
        consensus.newTargetTimespan = 3 * 60 * 60;              // 3 hour
        consensus.newTargetSpacingPoW = 2.5 * 60;               // 2.5 min 
        consensus.newTargetSpacingPoS = 5.5 * 60;               // 5.5 min 
        consensus.newLimitShift = 24;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfd;
        pchMessageStart[1] = 0xd2;
        pchMessageStart[2] = 0xc8;
        pchMessageStart[3] = 0x07;
        nDefaultPort = 18333;
        nPruneAfterHeight = 10000;
 
        genesis = CreateGenesisBlock(1317798646, 393879, 0x1e0ffff0, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        LogPrintf("test_found is %s\n", consensus.hashGenesisBlock.ToString());
        //assert(consensus.hashGenesisBlock ==
        //       uint256S("0xc079fd1ae86223e1522928776899d46e329da7919ca1e11be23643c67dd05d5f"));
        assert(genesis.hashMerkleRoot ==
               uint256S("0x985fae483ebbef9cde04a259282cb7465d52bf56824caf1a8132395e90488b12")); 

        vSeeds.clear();
        vFixedSeeds.clear();

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 196);
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94}; 
        
        bech32_hrp = "test";

        fDefaultConsistencyChecks = false;
        fRequireStandard = false;
        fMineBlocksOnDemand = false;

        checkpointData = {
            {
            }
        };

        chainTxData = ChainTxData{
            // Data from rpc: getchaintxstats 4096 0000000000000037a8cd3e06cd5edbfe9dd1dbcc5dacab279376ef7cfc2b4c75
            /* nTime    */ 1531929919,
            /* nTxCount */ 19438708,
            /* dTxRate  */ 0.626
        };

        /* enable fallback fee on testnet */
        m_fallback_fee_enabled = true;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        consensus.nSubsidyHalvingInterval = 150;
        consensus.BIP34Height = 100000000; // BIP34 has not activated on regtest (far in the future so block v1 are not rejected in tests)
        consensus.BIP34Hash = uint256();
        consensus.BIP65Height = 1351; // BIP65 activated on regtest (Used in rpc activation tests)
        consensus.BIP66Height = 1251; // BIP66 activated on regtest (Used in rpc activation tests)
        consensus.CSVHeight = 1400;
        consensus.WitnessHeight = 1400;
        consensus.powLimit = uint256S("7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowTargetTimespan = 14 * 24 * 60 * 60; // two weeks
        consensus.nPowTargetSpacingBegin = 10 * 60;
        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        // By default assume that the signatures in ancestors of this block are valid.
        consensus.defaultAssumeValid = uint256S("0x00");

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(1296688602, 2, 0x207fffff, 1, 50 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206"));
        assert(genesis.hashMerkleRoot == uint256S("0x4a5e1e4baab89f3a32518a88c31bc87f618f76673e2cc77ab2127b7afdeda33b"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;

        checkpointData = {
            {
                {0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")},
            }
        };

        chainTxData = ChainTxData{
            0,
            0,
            0
        };

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1,111);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1,196);
        base58Prefixes[SECRET_KEY] =     std::vector<unsigned char>(1,239);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "bcrt";

        /* enable fallback fee on regtest */
        m_fallback_fee_enabled = true;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params() {
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateVersionBitsParameters(Consensus::DeploymentPos d, int64_t nStartTime, int64_t nTimeout)
{
    globalChainParams->UpdateVersionBitsParameters(d, nStartTime, nTimeout);
}
