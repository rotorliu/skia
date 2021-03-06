/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGpu_DEFINED
#define GrGLGpu_DEFINED

#include "GrGLContext.h"
#include "GrGLIRect.h"
#include "GrGLIndexBuffer.h"
#include "GrGLPathRendering.h"
#include "GrGLProgram.h"
#include "GrGLRenderTarget.h"
#include "GrGLStencilAttachment.h"
#include "GrGLTexture.h"
#include "GrGLVertexArray.h"
#include "GrGLVertexBuffer.h"
#include "GrGpu.h"
#include "GrPipelineBuilder.h"
#include "GrXferProcessor.h"
#include "SkTypes.h"

class GrPipeline;

#ifdef SK_DEVELOPER
#define PROGRAM_CACHE_STATS
#endif

class GrGLGpu : public GrGpu {
public:
    GrGLGpu(const GrGLContext& ctx, GrContext* context);
    ~GrGLGpu() override;

    void contextAbandoned() override;

    const GrGLContext& glContext() const { return fGLContext; }

    const GrGLInterface* glInterface() const { return fGLContext.interface(); }
    const GrGLContextInfo& ctxInfo() const { return fGLContext; }
    GrGLStandard glStandard() const { return fGLContext.standard(); }
    GrGLVersion glVersion() const { return fGLContext.version(); }
    GrGLSLGeneration glslGeneration() const { return fGLContext.glslGeneration(); }
    const GrGLCaps& glCaps() const { return *fGLContext.caps(); }

    GrGLPathRendering* glPathRendering() {
        SkASSERT(glCaps().shaderCaps()->pathRenderingSupport());
        return static_cast<GrGLPathRendering*>(pathRendering());
    }

    void discard(GrRenderTarget*) override;

    // Used by GrGLProgram and GrGLPathTexGenProgramEffects to configure OpenGL
    // state.
    void bindTexture(int unitIdx, const GrTextureParams& params, GrGLTexture* texture);

    // GrGpu overrides
    GrPixelConfig preferredReadPixelsConfig(GrPixelConfig readConfig,
                                            GrPixelConfig surfaceConfig) const override;
    GrPixelConfig preferredWritePixelsConfig(GrPixelConfig writeConfig,
                                             GrPixelConfig surfaceConfig) const override;
    bool canWriteTexturePixels(const GrTexture*, GrPixelConfig srcConfig) const override;
    bool readPixelsWillPayForYFlip(GrRenderTarget* renderTarget,
                                   int left, int top,
                                   int width, int height,
                                   GrPixelConfig config,
                                   size_t rowBytes) const override;
    bool fullReadPixelsIsFasterThanPartial() const override;

    bool initCopySurfaceDstDesc(const GrSurface* src, GrSurfaceDesc* desc) override;

    // These functions should be used to bind GL objects. They track the GL state and skip redundant
    // bindings. Making the equivalent glBind calls directly will confuse the state tracking.
    void bindVertexArray(GrGLuint id) {
        fHWGeometryState.setVertexArrayID(this, id);
    }
    void bindIndexBufferAndDefaultVertexArray(GrGLuint id) {
        fHWGeometryState.setIndexBufferIDOnDefaultVertexArray(this, id);
    }
    void bindVertexBuffer(GrGLuint id) {
        fHWGeometryState.setVertexBufferID(this, id);
    }

    // These callbacks update state tracking when GL objects are deleted. They are called from
    // GrGLResource onRelease functions.
    void notifyVertexArrayDelete(GrGLuint id) {
        fHWGeometryState.notifyVertexArrayDelete(id);
    }
    void notifyVertexBufferDelete(GrGLuint id) {
        fHWGeometryState.notifyVertexBufferDelete(id);
    }
    void notifyIndexBufferDelete(GrGLuint id) {
        fHWGeometryState.notifyIndexBufferDelete(id);
    }

    bool copySurface(GrSurface* dst,
                     GrSurface* src,
                     const SkIRect& srcRect,
                     const SkIPoint& dstPoint) override;

    bool canCopySurface(const GrSurface* dst,
                        const GrSurface* src,
                        const SkIRect& srcRect,
                        const SkIPoint& dstPoint) override;

    void xferBarrier(GrXferBarrierType) override;

    void buildProgramDesc(GrProgramDesc*,
                          const GrPrimitiveProcessor&,
                          const GrPipeline&,
                          const GrBatchTracker&) const override;

private:
    // GrGpu overrides
    void onResetContext(uint32_t resetBits) override;

    GrTexture* onCreateTexture(const GrSurfaceDesc& desc, GrGpuResource::LifeCycle lifeCycle,
                               const void* srcData, size_t rowBytes) override;
    GrTexture* onCreateCompressedTexture(const GrSurfaceDesc& desc,
                                         GrGpuResource::LifeCycle lifeCycle,
                                         const void* srcData) override;
    GrVertexBuffer* onCreateVertexBuffer(size_t size, bool dynamic) override;
    GrIndexBuffer* onCreateIndexBuffer(size_t size, bool dynamic) override;
    GrTexture* onWrapBackendTexture(const GrBackendTextureDesc&) override;
    GrRenderTarget* onWrapBackendRenderTarget(const GrBackendRenderTargetDesc&) override;
    bool createStencilAttachmentForRenderTarget(GrRenderTarget* rt, int width, int height) override;
    bool attachStencilAttachmentToRenderTarget(GrStencilAttachment* sb,
                                               GrRenderTarget* rt) override;

    void onClear(GrRenderTarget*, const SkIRect* rect, GrColor color,
                 bool canIgnoreRect) override;

    void onClearStencilClip(GrRenderTarget*, const SkIRect& rect, bool insideClip) override;

    bool onReadPixels(GrRenderTarget* target,
                      int left, int top,
                      int width, int height,
                      GrPixelConfig,
                      void* buffer,
                      size_t rowBytes) override;

    bool onWriteTexturePixels(GrTexture* texture,
                              int left, int top, int width, int height,
                              GrPixelConfig config, const void* buffer,
                              size_t rowBytes) override;

    void onResolveRenderTarget(GrRenderTarget* target) override;

    void onDraw(const DrawArgs&, const GrDrawTarget::DrawInfo&) override;
    void onStencilPath(const GrPath*, const StencilPathState&) override;
    void onDrawPath(const DrawArgs&, const GrPath*, const GrStencilSettings&) override;
    void onDrawPaths(const DrawArgs&,
                     const GrPathRange*,
                     const void* indices,
                     GrDrawTarget::PathIndexType,
                     const float transformValues[],
                     GrDrawTarget::PathTransformType,
                     int count,
                     const GrStencilSettings&) override;

    void clearStencil(GrRenderTarget*) override;

    // GrDrawTarget overrides
    void didAddGpuTraceMarker() override;
    void didRemoveGpuTraceMarker() override;

    // binds texture unit in GL
    void setTextureUnit(int unitIdx);

    // Flushes state from GrPipeline to GL. Returns false if the state couldn't be set.
    bool flushGLState(const DrawArgs&);

    // Sets up vertex attribute pointers and strides. On return indexOffsetInBytes gives the offset
    // an into the index buffer. It does not account for drawInfo.startIndex() but rather the start
    // index is relative to the returned offset.
    void setupGeometry(const GrPrimitiveProcessor&,
                       const GrDrawTarget::DrawInfo& info,
                       size_t* indexOffsetInBytes);

    // Subclasses should call this to flush the blend state.
    void flushBlend(const GrXferProcessor::BlendInfo& blendInfo);

    bool hasExtension(const char* ext) const { return fGLContext.hasExtension(ext); }

    static bool BlendCoeffReferencesConstant(GrBlendCoeff coeff);

    class ProgramCache : public ::SkNoncopyable {
    public:
        ProgramCache(GrGLGpu* gpu);
        ~ProgramCache();

        void abandon();
        GrGLProgram* getProgram(const DrawArgs&);

    private:
        enum {
            // We may actually have kMaxEntries+1 shaders in the GL context because we create a new
            // shader before evicting from the cache.
            kMaxEntries = 128,
            kHashBits = 6,
        };

        struct Entry;

        struct ProgDescLess;

        // binary search for entry matching desc. returns index into fEntries that matches desc or ~
        // of the index of where it should be inserted.
        int search(const GrProgramDesc& desc) const;

        // sorted array of all the entries
        Entry*                      fEntries[kMaxEntries];
        // hash table based on lowest kHashBits bits of the program key. Used to avoid binary
        // searching fEntries.
        Entry*                      fHashTable[1 << kHashBits];

        int                         fCount;
        unsigned int                fCurrLRUStamp;
        GrGLGpu*                    fGpu;
#ifdef PROGRAM_CACHE_STATS
        int                         fTotalRequests;
        int                         fCacheMisses;
        int                         fHashMisses; // cache hit but hash table missed
#endif
    };

    void flushDither(bool dither);
    void flushColorWrite(bool writeColor);
    void flushDrawFace(GrPipelineBuilder::DrawFace face);

    // flushes the scissor. see the note on flushBoundTextureAndParams about
    // flushing the scissor after that function is called.
    void flushScissor(const GrScissorState&,
                      const GrGLIRect& rtViewport,
                      GrSurfaceOrigin rtOrigin);

    // disables the scissor
    void disableScissor();

    void initFSAASupport();

    // determines valid stencil formats
    void initStencilFormats();

    // sets a texture unit to use for texture operations other than binding a texture to a program.
    // ensures that such operations don't negatively interact with tracking bound textures.
    void setScratchTextureUnit();

    // bounds is region that may be modified and therefore has to be resolved.
    // NULL means whole target. Can be an empty rect.
    void flushRenderTarget(GrGLRenderTarget*, const SkIRect* bounds);

    void flushStencil(const GrStencilSettings&);
    void flushHWAAState(GrRenderTarget* rt, bool useHWAA);

    bool configToGLFormats(GrPixelConfig config,
                           bool getSizedInternal,
                           GrGLenum* internalFormat,
                           GrGLenum* externalFormat,
                           GrGLenum* externalType);
    // helper for onCreateTexture and writeTexturePixels
    bool uploadTexData(const GrSurfaceDesc& desc,
                       bool isNewTexture,
                       int left, int top, int width, int height,
                       GrPixelConfig dataConfig,
                       const void* data,
                       size_t rowBytes);

    // helper for onCreateCompressedTexture. If width and height are
    // set to -1, then this function will use desc.fWidth and desc.fHeight
    // for the size of the data. The isNewTexture flag should be set to true
    // whenever a new texture needs to be created. Otherwise, we assume that
    // the texture is already in GPU memory and that it's going to be updated
    // with new data.
    bool uploadCompressedTexData(const GrSurfaceDesc& desc,
                                 const void* data,
                                 bool isNewTexture = true,
                                 int left = 0, int top = 0,
                                 int width = -1, int height = -1);

    bool createRenderTargetObjects(const GrSurfaceDesc&, GrGpuResource::LifeCycle lifeCycle,
                                   GrGLuint texID, GrGLRenderTarget::IDDesc*);

    enum TempFBOTarget {
        kSrc_TempFBOTarget,
        kDst_TempFBOTarget
    };

    GrGLuint bindSurfaceAsFBO(GrSurface* surface, GrGLenum fboTarget, GrGLIRect* viewport,
                              TempFBOTarget tempFBOTarget);

    void unbindTextureFromFBO(GrGLenum fboTarget);

    GrGLContext fGLContext;

    // GL program-related state
    ProgramCache*               fProgramCache;
    SkAutoTUnref<GrGLProgram>   fCurrentProgram;

    ///////////////////////////////////////////////////////////////////////////
    ///@name Caching of GL State
    ///@{
    int                         fHWActiveTextureUnitIdx;
    GrGLuint                    fHWProgramID;

    enum TriState {
        kNo_TriState,
        kYes_TriState,
        kUnknown_TriState
    };

    GrGLuint                    fTempSrcFBOID;
    GrGLuint                    fTempDstFBOID;

    GrGLuint                    fStencilClearFBOID;

    // last scissor / viewport scissor state seen by the GL.
    struct {
        TriState    fEnabled;
        GrGLIRect   fRect;
        void invalidate() {
            fEnabled = kUnknown_TriState;
            fRect.invalidate();
        }
    } fHWScissorSettings;

    GrGLIRect   fHWViewport;

    /**
     * Tracks bound vertex and index buffers and vertex attrib array state.
     */
    class HWGeometryState {
    public:
        HWGeometryState() { fVBOVertexArray = NULL; this->invalidate(); }

        ~HWGeometryState() { SkSafeUnref(fVBOVertexArray); }

        void invalidate() {
            fBoundVertexArrayIDIsValid = false;
            fBoundVertexBufferIDIsValid = false;
            fDefaultVertexArrayBoundIndexBufferID = false;
            fDefaultVertexArrayBoundIndexBufferIDIsValid = false;
            fDefaultVertexArrayAttribState.invalidate();
            if (fVBOVertexArray) {
                fVBOVertexArray->invalidateCachedState();
            }
        }

        void notifyVertexArrayDelete(GrGLuint id) {
            if (fBoundVertexArrayIDIsValid && fBoundVertexArrayID == id) {
                // Does implicit bind to 0
                fBoundVertexArrayID = 0;
            }
        }

        void setVertexArrayID(GrGLGpu* gpu, GrGLuint arrayID) {
            if (!gpu->glCaps().vertexArrayObjectSupport()) {
                SkASSERT(0 == arrayID);
                return;
            }
            if (!fBoundVertexArrayIDIsValid || arrayID != fBoundVertexArrayID) {
                GR_GL_CALL(gpu->glInterface(), BindVertexArray(arrayID));
                fBoundVertexArrayIDIsValid = true;
                fBoundVertexArrayID = arrayID;
            }
        }

        void notifyVertexBufferDelete(GrGLuint id) {
            if (fBoundVertexBufferIDIsValid && id == fBoundVertexBufferID) {
                fBoundVertexBufferID = 0;
            }
            if (fVBOVertexArray) {
                fVBOVertexArray->notifyVertexBufferDelete(id);
            }
            fDefaultVertexArrayAttribState.notifyVertexBufferDelete(id);
        }

        void notifyIndexBufferDelete(GrGLuint id) {
            if (fDefaultVertexArrayBoundIndexBufferIDIsValid &&
                id == fDefaultVertexArrayBoundIndexBufferID) {
                fDefaultVertexArrayBoundIndexBufferID = 0;
            }
            if (fVBOVertexArray) {
                fVBOVertexArray->notifyIndexBufferDelete(id);
            }
        }

        void setVertexBufferID(GrGLGpu* gpu, GrGLuint id) {
            if (!fBoundVertexBufferIDIsValid || id != fBoundVertexBufferID) {
                GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ARRAY_BUFFER, id));
                fBoundVertexBufferIDIsValid = true;
                fBoundVertexBufferID = id;
            }
        }

        /**
         * Binds the default vertex array and binds the index buffer. This is used when binding
         * an index buffer in order to update it.
         */
        void setIndexBufferIDOnDefaultVertexArray(GrGLGpu* gpu, GrGLuint id) {
            this->setVertexArrayID(gpu, 0);
            if (!fDefaultVertexArrayBoundIndexBufferIDIsValid ||
                id != fDefaultVertexArrayBoundIndexBufferID) {
                GR_GL_CALL(gpu->glInterface(), BindBuffer(GR_GL_ELEMENT_ARRAY_BUFFER, id));
                fDefaultVertexArrayBoundIndexBufferIDIsValid = true;
                fDefaultVertexArrayBoundIndexBufferID = id;
            }
        }

        /**
         * Binds the vertex array object that should be used to render from the vertex buffer.
         * The vertex array is bound and its attrib array state object is returned. The vertex
         * buffer is bound. The index buffer (if non-NULL) is bound to the vertex array. The
         * returned GrGLAttribArrayState should be used to set vertex attribute arrays.
         */
        GrGLAttribArrayState* bindArrayAndBuffersToDraw(GrGLGpu* gpu,
                                                        const GrGLVertexBuffer* vbuffer,
                                                        const GrGLIndexBuffer* ibuffer);

    private:
        GrGLuint                fBoundVertexArrayID;
        GrGLuint                fBoundVertexBufferID;
        bool                    fBoundVertexArrayIDIsValid;
        bool                    fBoundVertexBufferIDIsValid;

        GrGLuint                fDefaultVertexArrayBoundIndexBufferID;
        bool                    fDefaultVertexArrayBoundIndexBufferIDIsValid;
        // We return a non-const pointer to this from bindArrayAndBuffersToDraw when vertex array 0
        // is bound. However, this class is internal to GrGLGpu and this object never leaks out of
        // GrGLGpu.
        GrGLAttribArrayState    fDefaultVertexArrayAttribState;

        // This is used when we're using a core profile and the vertices are in a VBO.
        GrGLVertexArray*        fVBOVertexArray;
    } fHWGeometryState;

    struct {
        GrBlendCoeff    fSrcCoeff;
        GrBlendCoeff    fDstCoeff;
        GrColor         fConstColor;
        bool            fConstColorValid;
        TriState        fEnabled;

        void invalidate() {
            fSrcCoeff = kInvalid_GrBlendCoeff;
            fDstCoeff = kInvalid_GrBlendCoeff;
            fConstColorValid = false;
            fEnabled = kUnknown_TriState;
        }
    } fHWBlendState;

    TriState fMSAAEnabled;

    GrStencilSettings           fHWStencilSettings;
    TriState                    fHWStencilTestEnabled;


    GrPipelineBuilder::DrawFace fHWDrawFace;
    TriState                    fHWWriteToColor;
    TriState                    fHWDitherEnabled;
    uint32_t                    fHWBoundRenderTargetUniqueID;
    SkTArray<uint32_t, true>    fHWBoundTextureUniqueIDs;

    ///@}

    // we record what stencil format worked last time to hopefully exit early
    // from our loop that tries stencil formats and calls check fb status.
    int fLastSuccessfulStencilFmtIdx;

    typedef GrGpu INHERITED;
    friend class GrGLPathRendering; // For accessing setTextureUnit.
};

#endif
