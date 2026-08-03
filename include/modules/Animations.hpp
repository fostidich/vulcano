// clang-format off
// NOLINTBEGIN(*)
#pragma once

#include "Starter.hpp"
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct AnimFrame {
    float time;
    glm::vec3 T;
    glm::quat Q;
    glm::vec3 S;
};

struct AnimTrack {
    int nKeyFrames;
    std::vector<AnimFrame> Frames;
    void getSampleTransforms(glm::vec3 &T, glm::quat &Q, glm::vec3 &S, float t, int sf, int ef, bool loop);
    glm::mat4 Sample(float t, int sf, int ef, bool loop);
    glm::mat4 Blend(float bf, float tinA, int sfA, int efA, float tinB, int sfB, int efB, AnimTrack *B = nullptr);
};

struct AnimBlendSegment {
    int st;
    int en;
    float t;
    int clip = 0;
};

struct AnimBlender {
    std::vector<AnimBlendSegment> segments;

    bool blending;
    int cur;
    int prev;
    float blendTime;
    float blendPos;

    void init(std::vector<AnimBlendSegment> seg);
    void Advance(float dt);
    void Start(int seg, float blendT);
    glm::mat4 Sample(AnimTrack *AT, AnimTrack *AT2 = nullptr);
    glm::mat4 Sample(std::vector<AnimTrack *> *AT);
};

class SkeletalAnimation;

class Animations {
    friend AssetFile;
    friend SkeletalAnimation;

    AssetFile *AF;
    std::unordered_map<std::string, AnimTrack *> GLTFanims;

  public:
    void init(AssetFile &A);
    void cleanup();
    AnimTrack *getAnim(std::string N);
};

class SkeletalAnimation {

    Animations *anims;
    tinygltf::Skin *skin;

    int NAnims;

    int NATs;
    std::vector<std::vector<AnimTrack *>> ATs;
    std::vector<int> ATsNodeId;
    int NTMs;
    std::vector<glm::mat4> oTMs;
    std::vector<glm::mat4> TMs;
    std::vector<glm::mat4> BaseTMs;
    std::vector<glm::mat4> IBMs;
    std::unordered_map<int, int> NidDec;

  public:
    void init(Animations *_anims, int _NAnims, std::string BaseTrackName, int SkinId = 0);
    void cleanup();
    std::vector<glm::mat4> *getTransformMatrices();
    void Sample(AnimBlender &AB);
    int getNTMs();
};

#ifdef ANIMATIONS_IMPLEMENTATION

AnimTrack *Animations::getAnim(std::string N) {
    return GLTFanims[N];
}

void AnimTrack::getSampleTransforms(glm::vec3 &T, glm::quat &Q, glm::vec3 &S, float tin, int sf, int ef, bool loop) {
    glm::vec3 T0, T1;
    glm::quat Q0, Q1;
    glm::vec3 S0, S1;

    if (ef < 0) {
        ef = ef + nKeyFrames + 1;
    }
    ef = ((ef < nKeyFrames) ? ef : nKeyFrames);

    float firstT = Frames[sf].time;
    float lastT = (ef >= nKeyFrames) ? 2 * Frames[nKeyFrames - 1].time - Frames[nKeyFrames - 2].time : Frames[ef].time;
    float interT = lastT - firstT;

    float t = fmod(tin, interT) + firstT;
    int srcl = sf, srcr = ef;
    while (srcl + 1 < srcr) {
        int srctst = (srcr + srcl) >> 1;
        logs::debug(srcl, " ", srctst, " ", srcr);
        if (t < Frames[srctst].time) {
            srcr = srctst;
        } else if (t > ((srctst + 1 < ef) ? Frames[srctst + 1].time : lastT)) {
            srcl = srctst + 1;
        } else {
            srcl = srcr = srctst;
        }
    }
    logs::debug("Found: ", srcl, " ", srcr, " ", Frames[srcl].time, " ", t, " ", ((srcl + 1 < ef) ? Frames[srcl + 1].time : lastT));

    int fi0 = srcl;
    int fi1 = (srcl + 1 < ef) ? (srcl + 1) : sf;

    AnimFrame F0 = Frames[fi0];
    T0 = F0.T;
    Q0 = F0.Q;
    S0 = F0.S;
    AnimFrame F1 = Frames[fi1];
    T1 = F1.T;
    Q1 = F1.Q;
    S1 = F1.S;
    float alpha = (t - F0.time) / (((fi0 + 1 < ef) ? F1.time : lastT) - F0.time);

    logs::debug("alpha: ", alpha);

    T = T0 * (1.0f - alpha) + T1 * alpha;
    Q = slerp(Q0, Q1, alpha);
    S = S0 * (1.0f - alpha) + S1 * alpha;
}

glm::mat4 AnimTrack::Sample(float tin, int sf = 0, int ef = -1, bool loop = false) {
    glm::mat4 out = glm::mat4(1);
    glm::vec3 T;
    glm::quat Q;
    glm::vec3 S;

    getSampleTransforms(T, Q, S, tin, sf, ef, loop);

    logs::debug(T.x, ", ", T.y, ", ", T.z, " || ",
         Q.x, ", ", Q.y, ", ", Q.z, ", ", Q.w, " || ",
         S.x, ", ", S.y, ", ", S.z);

    out = glm::translate(glm::mat4(1), T) *
          glm::mat4(Q) *
          glm::scale(glm::mat4(1), S);

    return out;
}

glm::mat4 AnimTrack::Blend(float bf, float tinA, int sfA, int efA, float tinB, int sfB, int efB, AnimTrack *B) {
    if (B == nullptr) {
        B = this;
    }
    glm::mat4 out = glm::mat4(1);
    glm::vec3 T, TA, TB;
    glm::quat Q, QA, QB;
    glm::vec3 S, SA, SB;

    getSampleTransforms(TA, QA, SA, tinA, sfA, efA, true);
    B->getSampleTransforms(TB, QB, SB, tinB, sfB, efB, true);

    T = TA * (1.0f - bf) + TB * bf;
    Q = slerp(QA, QB, bf);
    S = SA * (1.0f - bf) + SB * bf;

    out = glm::translate(glm::mat4(1), T) *
          glm::mat4(Q) *
          glm::scale(glm::mat4(1), S);

    return out;
}

void AnimBlender::init(std::vector<AnimBlendSegment> seg) {
    segments = seg;
    blending = false;
    cur = 0;
    prev = 0;
    blendTime = 0;
}

void AnimBlender::Advance(float dt) {
    if (blending) {
        segments[cur].t += dt;
        segments[prev].t += dt;
        blendPos += dt;
        if (blendPos > blendTime) {
            blending = false;
        }
    } else {
        segments[cur].t += dt;
    }
}

void AnimBlender::Start(int seg, float blendT) {
    if ((seg < segments.size()) && (seg != cur)) {
        prev = cur;
        cur = seg;
        segments[cur].t = 0;
        if (blendT > 0) {
            blendPos = 0;
            blendTime = blendT;
            blending = true;
        }
    }
}

glm::mat4 AnimBlender::Sample(AnimTrack *AT, AnimTrack *AT2) {
    if (AT2 == nullptr) {
        AT2 = AT;
    }
    if (blending) {
        return AT->Blend(1.0f - blendPos / blendTime, segments[cur].t, segments[cur].st, segments[cur].en, segments[prev].t, segments[prev].st, segments[prev].en, AT2);
    } else {
        return AT->Sample(segments[cur].t, segments[cur].st, segments[cur].en);
    }
}

glm::mat4 AnimBlender::Sample(std::vector<AnimTrack *> *AT) {
    return Sample((*AT)[segments[cur].clip], (*AT)[segments[prev].clip]);
}

void Animations::init(AssetFile &A) {
    AF = &A;

    if (A.getType() != GLTF) {
        logs::error("Error: Animations supported only in GLTF assets");
        exit(0);
    }

    tinygltf::Model *model = A.getGLTFmodel();

    for (const auto &anim : model->animations) {
        logs::debug(" Anim. Name:", anim.name, " Channels: ", anim.channels.size(), " Samplers: ", anim.samplers.size());

        std::unordered_map<int, const float *> Trans;
        std::unordered_map<int, const float *> Rot;
        std::unordered_map<int, const float *> Scale;
        const float *Time = nullptr;
        int nKeyFrames = 0;
        int targetNode;
        std::unordered_map<int, bool> nodeIds;

        // Find animation channels
        for (const auto &chan : anim.channels) {

            if (Time == nullptr) {
                const tinygltf::Accessor &inAccessor = model->accessors[anim.samplers[chan.sampler].input];
                const tinygltf::BufferView &inView = model->bufferViews[inAccessor.bufferView];
                const float *inVals = reinterpret_cast<const float *>(&(model->buffers[inView.buffer].data[inAccessor.byteOffset + inView.byteOffset]));
                int cntIn = inAccessor.count;

                Time = inVals;
                nKeyFrames = cntIn;
            }
            targetNode = chan.target_node;
            nodeIds[targetNode] = true;

            const tinygltf::Accessor &outAccessor = model->accessors[anim.samplers[chan.sampler].output];
            const tinygltf::BufferView &outView = model->bufferViews[outAccessor.bufferView];
            const float *outVals = reinterpret_cast<const float *>(&(model->buffers[outView.buffer].data[outAccessor.byteOffset + outView.byteOffset]));
            int cntOut = outAccessor.count;

            if (chan.target_path == "translation") {
                if (cntOut != nKeyFrames) {
                    logs::error("Number of Keyframes error: ", cntOut, " != ", nKeyFrames);
                    exit(0);
                }
                Trans[targetNode] = outVals;
            }

            if (chan.target_path == "rotation") {
                if (cntOut != nKeyFrames) {
                    logs::error("Number of Keyframes error: ", cntOut, " != ", nKeyFrames);
                    exit(0);
                }
                Rot[targetNode] = outVals;
            }

            if (chan.target_path == "scale") {
                if (cntOut != nKeyFrames) {
                    logs::error("Number of Keyframes error: ", cntOut, " != ", nKeyFrames);
                    exit(0);
                }
                Scale[targetNode] = outVals;
            }
        }

        logs::debug("There are ", nodeIds.size(), " animated nodes");
        for (auto tninf : nodeIds) {
            targetNode = tninf.first;

            std::ostringstream trackName;

            if (nodeIds.size() > 1) {
                trackName << anim.name << "#" << targetNode;
            } else {
                trackName << anim.name;
            }
            logs::debug(targetNode, ":", trackName.str());

            AnimTrack *AT = new AnimTrack();
            AT->nKeyFrames = nKeyFrames;
            // Create transform nodes
            for (int kf = 0; kf < nKeyFrames; kf++) {
                float kfTm = Time[kf];
                glm::vec3 T;
                glm::vec3 S;
                glm::quat Q;
                Model::getGLTFnodeTransforms(&model->nodes[targetNode], T, S, Q);
                if (Trans[targetNode] != nullptr) {
                    T = glm::vec3(Trans[targetNode][0], Trans[targetNode][1], Trans[targetNode][2]);
                    Trans[targetNode] += 3;
                }
                if (Rot[targetNode] != nullptr) {
                    Q = glm::quat(Rot[targetNode][3], Rot[targetNode][0], Rot[targetNode][1], Rot[targetNode][2]);
                    Rot[targetNode] += 4;
                }
                if (Scale[targetNode] != nullptr) {
                    S = glm::vec3(Scale[targetNode][0], Scale[targetNode][0], Scale[targetNode][0]);
                    Scale[targetNode] += 3;
                }
                AnimFrame A = {kfTm, T, Q, S};
                AT->Frames.push_back(A);
            }
            GLTFanims[trackName.str()] = AT;
        }
    }
}

void Animations::cleanup() {
    for (auto &a : GLTFanims) {
        delete a.second;
    }
}

void SkeletalAnimation::init(Animations *_anims, int _NAnims, std::string BaseTrackName, int SkinId) {
    anims = _anims;
    NAnims = _NAnims;

    tinygltf::Model *model;
    for (int naic = 0; naic < NAnims; naic++) {
        model = anims[naic].AF->getGLTFmodel();
        if (naic == 0) {
            logs::debug("Model has: ", model->skins.size(), " skins");
            skin = &model->skins[SkinId];

            int decPos = 0;
            for (auto &jid : skin->joints) {
                NidDec[jid] = decPos;
                decPos++;
            }
        } else {
            if (model->skins[SkinId].joints.size() != skin->joints.size()) {
                logs::error("Error! Animation ", naic, " has a different number of joints compared to Animation 0");
                exit(0);
            }
        }
    }
    oTMs.resize(skin->joints.size());
    TMs.resize(skin->joints.size());
    BaseTMs.resize(skin->joints.size());
    IBMs.resize(skin->joints.size());

    logs::debug("Base animation track name: ", BaseTrackName);

    for (int naic = 0; naic < NAnims; naic++) {
        model = anims[naic].AF->getGLTFmodel();
        if (naic == 0) {
            for (int i = 0; i < skin->joints.size(); i++) {
                std::ostringstream trackName;
                int targetNode;
                targetNode = skin->joints[i];
                trackName << BaseTrackName << "#" << targetNode;

                AnimTrack *at = anims[naic].getAnim(trackName.str());
                if (at != nullptr) {
                    ATs.push_back({});
                    ATs[ATs.size() - 1].push_back(at);
                    ATsNodeId.push_back(targetNode);
                } else {
                    glm::vec3 T;
                    glm::vec3 S;
                    glm::quat Q;
                    Model::getGLTFnodeTransforms(&model->nodes[targetNode], T, S, Q);
                    BaseTMs[targetNode] =
                        glm::translate(glm::mat4(1), T) *
                        glm::mat4(Q) *
                        glm::scale(glm::mat4(1), S);
                }
            }
        } else {
            int atsCorrI = 0;
            for (int i = 0; i < skin->joints.size(); i++) {
                std::ostringstream trackName;
                int targetNode;
                targetNode = skin->joints[i];
                trackName << BaseTrackName << "#" << targetNode;

                AnimTrack *at = anims[naic].getAnim(trackName.str());
                if (at != nullptr) {
                    ATs[atsCorrI].push_back(at);
                    if (ATsNodeId[atsCorrI] == targetNode) {
                        atsCorrI++;
                    } else {
                        logs::error("Error! Animation ", naic, " does not match Animation 0 skin structure");
                        exit(0);
                    }
                } else {
                    glm::vec3 T;
                    glm::vec3 S;
                    glm::quat Q;
                    Model::getGLTFnodeTransforms(&model->nodes[targetNode], T, S, Q);
                    BaseTMs[targetNode] =
                        glm::translate(glm::mat4(1), T) *
                        glm::mat4(Q) *
                        glm::scale(glm::mat4(1), S);
                    logs::debug(targetNode, " is not animated ");
                }
            }
        }
    }
    model = anims[0].AF->getGLTFmodel();

    logs::debug("found: ", ATs.size(), " matching tracks");
    NATs = ATs.size();
    NTMs = skin->joints.size();

    const tinygltf::Accessor &inAccessor = model->accessors[skin->inverseBindMatrices];
    const tinygltf::BufferView &inView = model->bufferViews[inAccessor.bufferView];
    const float *inVals = reinterpret_cast<const float *>(&(model->buffers[inView.buffer].data[inAccessor.byteOffset + inView.byteOffset]));

    for (int mel = 0; mel < NTMs; mel++) {
        const float *s = &inVals[mel * 16];
        IBMs[skin->joints[mel]] = glm::mat4(
            s[0], s[1], s[2], s[3],
            s[4], s[5], s[6], s[7],
            s[8], s[9], s[10], s[11],
            s[12], s[13], s[14], s[15]);
    }
}

void SkeletalAnimation::cleanup() {
}

std::vector<glm::mat4> *SkeletalAnimation::getTransformMatrices() {
    for (int i = 0; i < NTMs; i++) {
        oTMs[NidDec[i]] = TMs[i];
    }
    return &oTMs;
}

void SkeletalAnimation::Sample(AnimBlender &AB) {
    for (int i = 0; i < NATs; i++) {
        BaseTMs[ATsNodeId[i]] = AB.Sample(&ATs[i]);
        logs::debug(ATs[i][0]->nKeyFrames, " = ");
        logs::debug(i, ": nd :", ATsNodeId[i], " = ");
        for(int mi = 0; mi<16; mi++) {
            logs::debug(BaseTMs[ATsNodeId[i]] [mi%4][mi/4], ((mi%4 < 3) ? ", " : ""));
        }

        if(ATsNodeId[i] == 25) {
            logs::debug("Found Target");
            AnimTrack *ATT = ATs[i][0];
            for(int frm = 0; frm < ATT->Frames.size(); frm++) {
                logs::debug(ATT->Frames[frm].time, ": ",
                     ATT->Frames[frm].T.x, ", ", ATT->Frames[frm].T.y, ", ", ATT->Frames[frm].T.z, " <> ",
                     ATT->Frames[frm].Q.x, ", ", ATT->Frames[frm].Q.y, ", ", ATT->Frames[frm].Q.z, ", ", ATT->Frames[frm].Q.w, " <> ",
                     ATT->Frames[frm].S.x, ", ", ATT->Frames[frm].S.y, ", ", ATT->Frames[frm].S.z);
            }
        }
    }

    for (int i = 0; i < NTMs; i++) {
        TMs[i] = BaseTMs[i];
    }

    tinygltf::Model *model = anims[0].AF->getGLTFmodel();
    for (int i = 0; i < skin->joints.size(); i++) {
        int targetNode;
        targetNode = skin->joints[i];

        logs::debug(targetNode, " = ");
        for(int mi = 0; mi<16; mi++) {
            logs::debug(TMs[targetNode] [mi%4][mi/4], ((mi%4 < 3) ? ", " : ""));
        }

        for (int icc = 0; icc < model->nodes[targetNode].children.size(); icc++) {
            int childNode = model->nodes[targetNode].children[icc];
            TMs[childNode] = TMs[targetNode] * TMs[childNode];
        }
    }

    for (int i = 0; i < NTMs; i++) {
        TMs[i] = TMs[i] * IBMs[i];
        logs::debug(i, " <> ", model->nodes[i].name, ": ", (-IBMs[i][3][0]), " ", (-IBMs[i][3][1]), " ", (-IBMs[i][3][2]));
        logs::debug(i, " = ");
        for(int mi = 0; mi<16; mi++) {
            logs::debug(IBMs[i] [mi%4][mi/4], ((mi%4 < 3) ? ", " : ""));
        }
    }
    //	exit(0);
}

int SkeletalAnimation::getNTMs() {
    return NTMs;
}

#endif

// NOLINTEND(*)
// clang-format on
