/***************************************************************************
 # Copyright (c) 2015-23, NVIDIA CORPORATION. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without
 # modification, are permitted provided that the following conditions
 # are met:
 #  * Redistributions of source code must retain the above copyright
 #    notice, this list of conditions and the following disclaimer.
 #  * Redistributions in binary form must reproduce the above copyright
 #    notice, this list of conditions and the following disclaimer in the
 #    documentation and/or other materials provided with the distribution.
 #  * Neither the name of NVIDIA CORPORATION nor the names of its
 #    contributors may be used to endorse or promote products derived
 #    from this software without specific prior written permission.
 #
 # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS "AS IS" AND ANY
 # EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 # IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 # PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 # PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 # PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 # OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 # (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 # OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/
#pragma once
#include "Falcor.h"
#include "RenderGraph/RenderPass.h"
#include "Utils/Sampling/SampleGenerator.h"
#include <chrono>

using namespace Falcor;

class RTPM : public RenderPass
{
public:
    FALCOR_PLUGIN_CLASS(RTPM, "RTPM", "Insert pass description here.");

    static ref<RTPM> create(ref<Device> pDevice, const Properties& props) { return make_ref<RTPM>(pDevice, props); }

    RTPM(ref<Device> pDevice, const Properties& props);

    virtual Properties getProperties() const override;
    virtual RenderPassReflection reflect(const CompileData& compileData) override;
    virtual void compile(RenderContext* pRenderContext, const CompileData& compileData) override {}
    virtual void execute(RenderContext* pRenderContext, const RenderData& renderData) override;
    virtual void renderUI(Gui::Widgets& widget) override;
    virtual void setScene(RenderContext* pRenderContext, const ref<Scene>& pScene) override {}
    virtual bool onMouseEvent(const MouseEvent& mouseEvent) override { return false; }
    virtual bool onKeyEvent(const KeyboardEvent& keyEvent) override { return false; }

    //
    enum class TextureFormat
    {
        _8Bit = 0u,
        _16Bit = 1u,
        _32Bit = 2u
    };

    enum LightTexMode : uint32_t
    {
        power = 0u,
        area = 1u
    };

private:
    void parseProperties(const Properties& props);
    //
    /** Checks the timer. This is to stop the renderer for performance tests
     */
    void checkTimer();

    /** Writes the output times to file in mTimesOutputFilePath
     */
    void outputTimes();

    // All from HashPPM
    // Internal state
    ref<Scene> mpScene;                     ///< Current scene.
    ref<SampleGenerator> mpSampleGenerator; ///< GPU sample generator.

    // Constants
    const float kMinPhotonRadius = 0.0001f; ///< At radius 0.0001 Photons are still visible
    const float kCollectTMin = 0.000001f;   ///< non configurable constant for collection for now
    const float kCollectTMax = 0.000002f;   ///< non configurable constant for collection for now
    const uint kInfoTexHeight = 512;        ///< Height of the info tex as it is too big for 1D tex

    //***************************************************************************
    // Configuration
    //***************************************************************************

    bool mUseStatisticProgressivePM = true; ///< Activate Statistically Progressive Photon Mapping(SPPM)
    float mSPPMAlphaGlobal = 0.7f;          ///< Global Alpha for SPPM
    float mSPPMAlphaCaustic = 0.7f;         ///< Caustic Alpha for SPPM

    float mCausticRadiusStart = 0.01f; ///< Start value for the caustic Radius
    float mGlobalRadiusStart = 0.05f;  ///< Start value for the caustic Radius
    float mCausticRadius = 1.f;        ///< Current Radius for caustic Photons
    float mGlobalRadius = 1.f;         ///< Current Radius for global Photons

    float mSpecRoughCutoff = 0.5f; ///< If rougness is over this value interpret the material as diffuse

    bool mResetIterations = false;       ///< Resets the iterations counter once
    bool mAlwaysResetIterations = false; ///< Resets the iteration counter every frame

    bool mNumPhotonsChanged = false;      ///< If true buffers needs to be restarted and Number of photons needs to be changed
    bool mFitBuffersToPhotonShot = false; ///< Changes the buffer size to be around the number of photons shot

    bool mUseAlphaTest = true;         ///< Uses alpha test (Generate)
    bool mAdjustShadingNormals = true; ///< Adjusts the shading normals (Generate)

    uint mNumBucketBits = 20;            ///< 2^NumBucketBits is the total amount of possible buckets
    uint mNumPhotonsPerBucket = 12;      ///< Max Photons per hash grid.
    uint mQuadraticProbeIterations = 10; ///< Number of quadartic probe iteratons per hash.

    bool mEnableFaceNormalRejection = false;

    // Generate only
    uint mMaxBounces = 10;         ///< Depth of recursion (0 = none).
    float mRussianRoulette = 0.3f; ///< Probabilty that a Global photon is saved

    uint mNumPhotons = 2000000;                  ///< Number of Photons shot
    uint mNumPhotonsUI = mNumPhotons;            ///< For UI. It is decopled from the runtime var because changes have to be confirmed
    uint mGlobalBufferSizeUI = mNumPhotons / 2;  ///< Size of the Global Photon Buffer
    uint mCausticBufferSizeUI = mNumPhotons / 4; ///< Size of the Caustic Photon Buffer

    uint mCausticMapMultipleDiffuseHits = 0; ///< Allows for L(S|D)*SD paths to be stored in the photon map. Treated like a bool
    float mIntensityScalar = 1.0f;           ///< Scales the intensity of emissive light sources

    // Collect only
    bool mDisableGlobalCollection = false;  ///< Disabled the collection of global photons
    bool mDisableCausticCollection = false; ///< Disabled the collection of caustic photons

    bool mEnableStochasticCollection = true;     ///< Enables/Disables Stochasic collection
    float mStochasticCollectProbability = 0.33f; ///< Probability for collection

    //*******************************************************
    // Runtime data
    //*******************************************************

    uint mFrameCount = 0; ///< Frame count since last Reset
    std::vector<uint> mPhotonCount = {0, 0};
    bool mOptionsChanged = false;
    bool mResetCS = true;
    bool mSetConstantBuffers = true;
    bool mResizePhotonBuffers = true; ///< If true resize the Photon Buffers
    bool mPhotonInfoFormatChanged = false;
    bool mRebuildAS = false;
    uint mInfoTexFormat = 1;
    uint mNumBuckets = 0;

    // Light
    std::vector<uint> mActiveEmissiveTriangles;
    bool mRebuildLightTex = false;
    LightTexMode mLightTexMode = LightTexMode::power;
    ref<Texture> mLightSampleTex;
    ref<Buffer> mPhotonsPerTriangle;
    const uint mMaxDispatchY = 512;
    uint mPGDispatchX = 0;
    uint mAnalyticEndIndex = 0;
    uint mNumLights = 0;
    float mAnalyticInvPdf = 0.0f;

    // Clock/Timer
    bool mUseTimer = false;                                             //<Activates the timer
    bool mResetTimer = false;                                           //<Resets the timer
    bool mTimerStopRenderer = false;                                    //<Stops rendering (via return)
    double mTimerDurationSec = 60.0;                                    //<How long the timer is running
    uint mTimerMaxIterations = 0;                                       //< Stop at certain iterations
    double mCurrentElapsedTime = 0.0;                                   //<Elapsed time for UI
    std::chrono::time_point<std::chrono::steady_clock> mTimerStartTime; //<Start time for the timer
    bool mTimerRecordTimes = false;                                     //< Enable Records times
    std::vector<double> mTimesList;                                     //< List with render times
    std::string mTimesOutputFilePath;                                   //< Output file path for the times

    // Ray tracing program.
    struct RayTraceProgramHelper
    {
        ref<Program> pProgram;
        ref<RtBindingTable> pBindingTable;
        ref<RtProgramVars> pVars;

        static const RayTraceProgramHelper create()
        {
            RayTraceProgramHelper r;
            r.pProgram = nullptr;
            r.pBindingTable = nullptr;
            r.pVars = nullptr;
            return r;
        }
    };

    ref<ComputePass> mpCSCollect;          ///< Collect pass collects the photons that where shot
    RayTraceProgramHelper mTracerGenerate; ///< Description for the Generate Photon pass

    //
    // Photon Buffers
    //

    // Struct for the buffers that are needed for global and caustic photons
    bool mPhotonBuffersReady = false;

    bool mTestInit = false;

    struct
    {
        ref<Buffer> counter;
        ref<Buffer> reset;
        ref<Buffer> cpuCopy;
    } mPhotonCounterBuffer;

    struct PhotonBuffers
    {
        uint maxSize = 0;
        ref<Texture> position;
        ref<Texture> infoFlux;
        ref<Texture> infoDir;
    };

    ref<Buffer> mpGlobalBuckets;
    ref<Buffer> mpCausticBuckets;

    PhotonBuffers mCausticBuffers; ///< Buffers for the caustic photons
    PhotonBuffers mGlobalBuffers;  ///< Buffers for the global photons

    ref<Texture> mRandNumSeedBuffer; ///< Buffer for the random seeds
};
