#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/binding/HardStreak.hpp>
#include <Geode/utils/cocos.hpp>

#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include <cstring>

/*
                                                /~~\
  ____                                         /'o  |
.';;|;;\            _,-;;;\;-_               ,'  _/'|
`\_/;;;/;\         /;;\;;;;\;;;,             |     .'
    `;/;;;|      ,;\;;;|;;;|;;;|;\          ,';;\  |
     |;;;/;:     |;;;\;/~~~~\;/;;;|        ,;;;;;;.'
     |;/;;;|     |;;;,'      `\;;/;|      /;\;;;;/
      `|;;;/;\___/;~\;|         |;;;;;----\;;;|;;/'
       `;/;;;|;;;|;;;,'         |;;;;|;;;;;|;;|/'
        `\;;;|;;;/;;,'           `\;/;;;;;;|/~'
          `\/;;/;;;/               `~------'
            `~~~~~
worm by chev
*/

using namespace geode::prelude;
using namespace cocos2d;

extern int gWormMode;
extern int gWormHistory;

static bool gInvisible = true;

class $modify(InvisiblePlayer, PlayerObject) {
public:
    struct Fields {
        int maxHistory = 90;
        std::deque<CCPoint> history;

        std::vector<CCSprite*> pool1;
        std::vector<CCSprite*> pool2;
        bool poolsReady = false;

        int altFrames1 = 8;
        int altFrames2 = 5;
        int stripeOffset = 0;

        CCSprite* head = nullptr;
        bool headReady = false;
        float headScale = 0.36f;

        CCSprite* mouth = nullptr;
        bool mouthReady = false;
        CCSprite* mouthFlipped = nullptr;
        bool mouthFlippedReady = false;
        float mouthScale = 0.1f;
        float mouthOffsetX = 10.0f;
        float mouthOffsetY = -5.0f;

        CCSprite* eye = nullptr;
        bool eyeReady = false;
        float eyeScale = 0.07f;
        float eyeOffsetX = 6.0f;
        float eyeOffsetY = 6.0f;

        float trailScale = 0.33f;
        int   zOffset = -1;
    };

    void resetObject() {
        PlayerObject::resetObject();

        //if not in worm mode dont display worm
        if (gWormMode <= 0) {
            clearOverlays();
            this->setVisible(true);
            this->setOpacity(255);
            return;
        }

        //in worm mode DO display worm, dont display non-worm things
        clearOverlays();
        disableKnownPlayerParticles();
        hidePlayerTrails();
    }

    void update(float dt) {
        PlayerObject::update(dt);

        //not worm mode:
        if (gWormMode <= 0) {
            hideAll();
            this->setVisible(true);
            this->setOpacity(255);
            return;
        }

        //worm mode:
        disableKnownPlayerParticles();
        hidePlayerTrails();

        if (!gInvisible) {
            this->setVisible(true);
            this->setOpacity(255);
            hideAll();
            return;
        }

        this->setVisible(false);
        this->setOpacity(0);
        if (!this->isCascadeOpacityEnabled())
            this->setCascadeOpacityEnabled(true);

        // keep trail history in sync with UI (90 for worm, 150 for big worm)
        syncHistoryLengthWithUI();

        ensureAllSprites();

        //record player position so we can draw the worm segments there
        const CCPoint cur = this->getPosition();
        m_fields->history.push_back(cur);
        if ((int)m_fields->history.size() > m_fields->maxHistory)
            m_fields->history.pop_front();

        //flip worm face for gravity and scale for mini
        float gMult = this->m_isUpsideDown ? -1.0f : 1.0f;
        float sx = this->getScaleX();
        float sy = this->getScaleY();
        bool isMini = (sx < 0.75f) || (sy < 0.75f);
        float miniMult = isMini ? 0.5f : 1.0f;

        //get player colors
        ccColor3B col1 = this->m_playerColor1;
        ccColor3B col2 = this->m_playerColor2;

        //worm stripes
        const int n = m_fields->maxHistory;
        const int h = (int)m_fields->history.size();
        int a1 = std::max(0, m_fields->altFrames1);
        int a2 = std::max(0, m_fields->altFrames2);
        int period = a1 + a2;
        if (period <= 0) period = 1;

        //change color of worm parts
        for (auto* s : m_fields->pool1) if (s) s->setColor(col1);
        for (auto* s : m_fields->pool2) if (s) s->setColor(col2);
        if (m_fields->head) m_fields->head->setColor(col1);
        if (m_fields->eye)  m_fields->eye->setColor({ 0, 0, 0 });

        //draw worm body, calculate offset for each circle based off of position history list
        for (int i = 0; i < n; ++i) {
            CCSprite* s1 = m_fields->pool1[i];
            CCSprite* s2 = m_fields->pool2[i];

            if (i < h) {
                int idxFromHead = (h - 1) - i;
                int posInPeriod = (m_fields->stripeOffset + (idxFromHead % period) + period) % period;
                bool useBody1 = (a2 == 0) || (posInPeriod < a1);

                const CCPoint p = m_fields->history[i];
                CCSprite* show = useBody1 ? s1 : s2;
                CCSprite* hide = useBody1 ? s2 : s1;

                if (hide) hide->setVisible(false);
                if (show) {
                    show->setVisible(true);
                    show->setPosition(p);
                    show->setRotation(0.0f);
                    show->setScale(m_fields->trailScale * miniMult);
                }
            }
            else {
                if (s1) s1->setVisible(false);
                if (s2) s2->setVisible(false);
            }
        }

        //worm head
        if (m_fields->head) {
            m_fields->head->setVisible(true);
            m_fields->head->setPosition(cur);
            m_fields->head->setRotation(0.0f);
            m_fields->head->setScale(m_fields->headScale * miniMult);
        }

        //worm mouth
        if (m_fields->mouth || m_fields->mouthFlipped) {
            CCSprite* show = this->m_isUpsideDown ? m_fields->mouthFlipped : m_fields->mouth;
            CCSprite* hide = this->m_isUpsideDown ? m_fields->mouth : m_fields->mouthFlipped;

            if (hide) hide->setVisible(false);
            if (show) {
                show->setVisible(true);
                show->setPosition(ccp(
                    cur.x + m_fields->mouthOffsetX * miniMult,
                    cur.y + (m_fields->mouthOffsetY * gMult * miniMult)
                ));
                show->setRotation(0.0f);
                show->setScale(m_fields->mouthScale * miniMult);
            }
        }

        //worm eye
        if (m_fields->eye) {
            m_fields->eye->setVisible(true);
            m_fields->eye->setPosition(ccp(
                cur.x + m_fields->eyeOffsetX * miniMult,
                cur.y + (m_fields->eyeOffsetY * gMult * miniMult)
            ));
            m_fields->eye->setRotation(0.0f);
            m_fields->eye->setScale(m_fields->eyeScale * miniMult);
        }
    }

private:
    void hideAll() {
        if (m_fields->poolsReady) {
            for (auto* s : m_fields->pool1) if (s) s->setVisible(false);
            for (auto* s : m_fields->pool2) if (s) s->setVisible(false);
        }
        if (m_fields->head)          m_fields->head->setVisible(false);
        if (m_fields->mouth)         m_fields->mouth->setVisible(false);
        if (m_fields->mouthFlipped)  m_fields->mouthFlipped->setVisible(false);
        if (m_fields->eye)           m_fields->eye->setVisible(false);
    }

    void clearOverlays() {
        m_fields->history.clear();
        hideAll();
    }

    void destroyPools() {
        if (!m_fields->poolsReady) return;
        auto* parent = this->getParent();

        for (auto* s : m_fields->pool1) {
            if (s && s->getParent()) s->removeFromParentAndCleanup(false);
        }
        for (auto* s : m_fields->pool2) {
            if (s && s->getParent()) s->removeFromParentAndCleanup(false);
        }
        m_fields->pool1.clear();
        m_fields->pool2.clear();

        if (m_fields->head && m_fields->head->getParent())
            m_fields->head->removeFromParentAndCleanup(false);
        if (m_fields->mouth && m_fields->mouth->getParent())
            m_fields->mouth->removeFromParentAndCleanup(false);
        if (m_fields->mouthFlipped && m_fields->mouthFlipped->getParent())
            m_fields->mouthFlipped->removeFromParentAndCleanup(false);
        if (m_fields->eye && m_fields->eye->getParent())
            m_fields->eye->removeFromParentAndCleanup(false);

        m_fields->head = nullptr;         m_fields->headReady = false;
        m_fields->mouth = nullptr;        m_fields->mouthReady = false;
        m_fields->mouthFlipped = nullptr; m_fields->mouthFlippedReady = false;
        m_fields->eye = nullptr;          m_fields->eyeReady = false;

        m_fields->poolsReady = false;
    }

    void syncHistoryLengthWithUI() {
        int desired = std::max(1, gWormHistory);
        if (m_fields->maxHistory != desired) {
            m_fields->maxHistory = desired;
            clearOverlays();
            destroyPools();
        }
    }

    static void stopAndHide(CCParticleSystemQuad * ps) {
        if (!ps) return;
        ps->stopSystem();
        ps->setVisible(false);
        ps->setEmissionRate(0);
        ps->setAutoRemoveOnFinish(false);
    }

    void disableKnownPlayerParticles() {
        stopAndHide(this->m_playerGroundParticles);
        stopAndHide(this->m_trailingParticles);
        stopAndHide(this->m_shipClickParticles);
        stopAndHide(this->m_vehicleGroundParticles);
        stopAndHide(this->m_ufoClickParticles);
        stopAndHide(this->m_robotBurstParticles);
        stopAndHide(this->m_dashParticles);
        stopAndHide(this->m_swingBurstParticles1);
        stopAndHide(this->m_swingBurstParticles2);
        stopAndHide(this->m_landParticles0);
        stopAndHide(this->m_landParticles1);
    }

    static void hideStreak(CCMotionStreak * s) {
        if (!s) return;
        s->setVisible(false);
        s->reset();
        s->setOpacity(0);
    }

    static void hideIfStreakish(CCNode * n) {
        if (!n) return;

        if (auto* ms = typeinfo_cast<CCMotionStreak*>(n)) {
            hideStreak(ms);
            return;
        }
        if (auto* hs = typeinfo_cast<HardStreak*>(n)) {
            hs->setVisible(false);
            return;
        }
    }

    void hideStreaksUnder(CCNode * node) {
        if (!node) return;
        auto* arr = node->getChildren();
        if (!arr) return;
        for (auto* ch : CCArrayExt<CCNode*>(arr)) {
            if (!ch) continue;
            hideIfStreakish(ch);
        }
    }

    void hidePlayerTrails() {
        hideStreaksUnder(this);
        if (auto* p = this->getParent())
            hideStreaksUnder(p);
    }

    void ensureAllSprites() {
        CCNode* parent = this->getParent();
        if (!parent) return;
        const int baseZ = this->getZOrder() + m_fields->zOffset;

        if (!m_fields->poolsReady) {
            m_fields->pool1.resize(m_fields->maxHistory);
            m_fields->pool2.resize(m_fields->maxHistory);

            for (int i = 0; i < m_fields->maxHistory; ++i) {
                CCSprite* a = CCSprite::create("circle.png"_spr);
                if (a) {
                    a->setAnchorPoint(ccp(0.5f, 0.5f));
                    a->setScale(m_fields->trailScale);
                    a->setVisible(false);
                    parent->addChild(a, baseZ + i);
                }
                m_fields->pool1[i] = a;

                CCSprite* b = CCSprite::create("circle.png"_spr);
                if (b) {
                    b->setAnchorPoint(ccp(0.5f, 0.5f));
                    b->setScale(m_fields->trailScale);
                    b->setVisible(false);
                    parent->addChild(b, baseZ + i);
                }
                m_fields->pool2[i] = b;
            }
            m_fields->poolsReady = true;
        }

        if (!m_fields->headReady) {
            CCSprite* h = CCSprite::create("circle.png"_spr);
            if (h) {
                h->setAnchorPoint(ccp(0.5f, 0.5f));
                h->setScale(m_fields->headScale);
                h->setVisible(false);
                parent->addChild(h, baseZ + m_fields->maxHistory + 5);
                m_fields->head = h;
                m_fields->headReady = true;
            }
        }

        if (!m_fields->mouthReady) {
            CCSprite* m = CCSprite::create("mouth.png"_spr);
            if (m) {
                m->setAnchorPoint(ccp(0.5f, 0.5f));
                m->setScale(m_fields->mouthScale);
                m->setVisible(false);
                parent->addChild(m, baseZ + m_fields->maxHistory + 6);
                m_fields->mouth = m;
                m_fields->mouthReady = true;
            }
        }

        if (!m_fields->mouthFlippedReady) {
            //yea I made a flipped version of the sprite instead of flipping it with code, dont post this in #dev-chat
            CCSprite* mf = CCSprite::create("mouth_flipped.png"_spr);
            if (mf) {
                mf->setAnchorPoint(ccp(0.5f, 0.5f));
                mf->setScale(m_fields->mouthScale);
                mf->setVisible(false);
                parent->addChild(mf, baseZ + m_fields->maxHistory + 6);
                m_fields->mouthFlipped = mf;
                m_fields->mouthFlippedReady = true;
            }
        }

        if (!m_fields->eyeReady) {
            CCSprite* e = CCSprite::create("circle.png"_spr);
            if (e) {
                e->setAnchorPoint(ccp(0.5f, 0.5f));
                e->setScale(m_fields->eyeScale);
                e->setVisible(false);
                parent->addChild(e, baseZ + m_fields->maxHistory + 7);
                e->setColor({ 0, 0, 0 });
                m_fields->eye = e;
                m_fields->eyeReady = true;
            }
        }
    }
};
