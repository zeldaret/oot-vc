#ifndef RVL_SDK_HBM_HOMEBUTTON_GUI_MANAGER_HPP
#define RVL_SDK_HBM_HOMEBUTTON_GUI_MANAGER_HPP

/*******************************************************************************
 * headers
 */

#include "macros.h"
#include "revolution/hbm/nw4hbm/lyt/pane.h"
#include "revolution/hbm/nw4hbm/ut/list.h"
#include "revolution/kpad/KPAD.h"
#include "revolution/mem/mem_allocator.h"
#include "revolution/mtx/mtx.h"
#include "revolution/mtx/vec.h"
#include "revolution/types.h"

/*******************************************************************************
 * classes and functions
 */

// forward declarations
namespace nw4hbm {
namespace lyt {
class DrawInfo;
class Layout;
} // namespace lyt
} // namespace nw4hbm

namespace homebutton {
namespace gui {
// forward declarations
class Manager;
class PaneComponent;

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x477921
class Interface {
    // methods
  public:
    // cdtors
    Interface() {}
    /* virtual ~Interface() {} */

    // virtual function ordering
    // vtable Interface
    virtual void create() {}
    virtual void init() {}
    virtual void calc() {}
    virtual void draw(Mtx&) {}
    virtual void draw() {}
    virtual ~Interface() {}

    // members
  private:
    /* vtable */ // size 0x04, offset 0x00
}; // size 0x04

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x4779b0
class EventHandler // also see HBMBase.cpp
{
    // methods
  public:
    // cdtors
    EventHandler() {}

    // virtual function ordering
    // vtable EventHandler
    virtual void onEvent(u32, u32, void*) {}
    virtual void setManager(Manager* pManager) { mpManager = pManager; }

    // members
  protected: // HomeButtonEventHandler::onEvent
    /* vtable */ // size 0x04, offset 0x00
    Manager* mpManager; // size 0x04, offset 0x04
}; // size 0x08

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x47786a
class Component : public Interface {
    // NOTE the misspelling of triggerTarget as triggerTarger

    // methods
  public:
    // cdtors
    Component(u32 uID) : mDragStartPos(), mbDragging(), muDraggingButton(), muID(uID), mbTriggerTarger(), mpManager() {
        init();
    }
    /* virtual ~Component(); */

    // virtual function ordering
    // vtable Interface
    virtual void init() {
        mbDragging = false;

        for (int i = 0; i < (int)ARRAY_COUNT(mabPointed); i++) {
            mabPointed[i] = false;
        }
    }

    virtual ~Component() {}

    // vtable Component
    virtual u32 getID() { return muID; }
    virtual int isPointed(int n) { return mabPointed[n]; }
    virtual void setPointed(int n, bool b) { mabPointed[n] = b; }
    virtual void onPoint() {}
    virtual void offPoint() {}
    virtual void onDrag(f32, f32) {}
    virtual void onMove(f32, f32) {}
    virtual void onTrig(u32 uFlag, Vec& vec) {
        if (uFlag & muDraggingButton) {
            mDragStartPos = vec;
            mbDragging = true;
        }
    }
    virtual void setDraggingButton(u32 uDraggingButton) { muDraggingButton = uDraggingButton; }
    virtual bool update(int, const KPADStatus*, f32, f32, void*) { return false; }
    virtual bool update(int i, f32 x, f32 y, u32 uTrigFlag, u32 uHoldFlag, u32 uReleaseFlag, void* pData);
    virtual bool isTriggerTarger() { return mbTriggerTarger; }
    virtual void setTriggerTarget(bool bTriggerTarget) { mbTriggerTarger = bTriggerTarget; }
    virtual void setManager(Manager* pManager) { mpManager = pManager; }
    virtual bool isVisible() { return true; }
    virtual bool contain(f32 x_, f32 y_) = 0;

    // members
  protected: // PaneComponent::contain
    /* base Interface */ // size 0x04, offset 0x00
    bool mabPointed[8]; // size 0x08, offset 0x04
    Vec mDragStartPos; // size 0x0c, offset 0x0c
    bool mbDragging; // size 0x01, offset 0x18
    /* 3 bytes padding */
    u32 muDraggingButton; // size 0x04, offset 0x1c
    u32 muID; // size 0x04, offset 0x20
    bool mbTriggerTarger; // size 0x01, offset 0x24
    /* 3 bytes padding */
    Manager* mpManager; // size 0x04, offset 0x28
}; // size 0x2c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x477947
class Manager : public Interface {
    // nested types
  private:
    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x477a9a
    struct IDToComponent {
        // methods
      public:
        // cdtors
        IDToComponent(u32 uID, Component* pComponent) : muID(uID), mpComponent(pComponent) {}

        // members
      public:
        u32 muID; // size 0x04, offset 0x00
        Component* mpComponent; // size 0x04, offset 0x04
        nw4hbm::ut::Link mLink; // size 0x08, offset 0x08
    }; // size 0x10

    // methods
  public:
    // cdtors
    Manager(EventHandler* pEventHandler, MEMAllocator* pAllocator)
        : mpEventHandler(pEventHandler), mpAllocator(pAllocator) {
        if (mpEventHandler) {
            mpEventHandler->setManager(this);
        }

        nw4hbm::ut::List_Init(&mIDToComponent, 8);
    }
    /* virtual ~Manager(); */

    // virtual function ordering
    // vtable Interface
    virtual void init();
    virtual void calc();
    virtual void draw();
    virtual ~Manager();

    // vtable Manager
    virtual void addComponent(Component* pComponent);
    virtual void delComponent(Component* pComponent);
    virtual Component* getComponent(u32 uID);
    virtual bool update(int, const KPADStatus*, f32, f32, void*) { return false; }
    virtual void onEvent(u32 uID, u32 uEvent, void* pData) {
        if (mpEventHandler) {
            mpEventHandler->onEvent(uID, uEvent, pData);
        }
    }
    virtual bool update(int i, f32 x, f32 y, u32 uTrigFlag, u32 uHoldFlag, u32 uReleaseFlag, void* pData);

    virtual void setAllComponentTriggerTarget(bool b);
    virtual void setEventHandler(EventHandler* pEventHandler) {
        mpEventHandler = pEventHandler;

        if (mpEventHandler) {
            mpEventHandler->setManager(this);
        }
    }

    // members
  protected: // PaneManager::~PaneManager
    /* base Interface */ // size 0x04, offset 0x00
    EventHandler* mpEventHandler; // size 0x04, offset 0x04
    nw4hbm::ut::List mIDToComponent; // size 0x0c, offset 0x08
    MEMAllocator* mpAllocator; // size 0x04, offset 0x14
}; // size 0x18

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x477bec
class PaneManager : public Manager {
    // nested types
  private:
    // [SGLEA4]/GormitiDebug.elf:.debug_info::0x477de8
    struct PaneToComponent {
        // methods
      public:
        // cdtors
        PaneToComponent(nw4hbm::lyt::Pane* pPane, PaneComponent* pComponent) : mpPane(pPane), mpComponent(pComponent) {}

        // members
      public:
        nw4hbm::lyt::Pane* mpPane; // size 0x04, offset 0x00
        PaneComponent* mpComponent; // size 0x04, offset 0x04
        nw4hbm::ut::Link mLink; // size 0x08, offset 0x08
    }; // size 0x10

    // methods
  public:
    // cdtors
    PaneManager(EventHandler* pEventHandler, const nw4hbm::lyt::DrawInfo* pDrawInfo, MEMAllocator* pAllocator)
        : Manager(pEventHandler, pAllocator), mpDrawInfo(pDrawInfo) {
        nw4hbm::ut::List_Init(&mPaneToComponent, 8);
    }
    /* virtual ~PaneManager(); */

    // virtual function ordering
    // vtable Interface
    virtual ~PaneManager();

    // vtable Manager

    // vtable PaneManager
    virtual void createLayoutScene(const nw4hbm::lyt::Layout& rLayout);
    virtual const nw4hbm::lyt::DrawInfo* getDrawInfo() { return mpDrawInfo; }
    virtual void delLayoutScene(const nw4hbm::lyt::Layout& rLayout);
    virtual PaneComponent* getPaneComponentByPane(nw4hbm::lyt::Pane* pPane);
    virtual void walkInChildren(nw4hbm::lyt::Pane::LinkList& rPaneList);
    virtual void addLayoutScene(const nw4hbm::lyt::Layout& rLayout);
    virtual void setDrawInfo(const nw4hbm::lyt::DrawInfo* pDrawInfo) { mpDrawInfo = pDrawInfo; }
    virtual void setAllBoundingBoxComponentTriggerTarget(bool b);
    virtual void walkInChildrenDel(nw4hbm::lyt::Pane::LinkList& rPaneList);

    // members
  private:
    /* base Manager */ // size 0x18, offset 0x00
    nw4hbm::ut::List mPaneToComponent; // size 0x0c, offset 0x18
    const nw4hbm::lyt::DrawInfo* mpDrawInfo; // size 0x04, offset 0x24
    u16 muNumPoint; // size 0x02, offset 0x28
    byte2_t muPadding;

    // static members
    static u32 suIDCounter;
}; // size 0x2c

// [SGLEA4]/GormitiDebug.elf:.debug_info::0x477e42
class PaneComponent : public Component {
    // methods
  public:
    // cdtors
    PaneComponent(u32 uID) : Component(uID), mpPane() {}
    /* virtual ~PaneComponent() {} */

    // virtual function ordering
    // vtable Interface
    virtual void draw();
    virtual ~PaneComponent() {}

    // vtable Component
    virtual bool isVisible();
    virtual bool contain(f32 x_, f32 y_);

    // vtable PaneComponent
    virtual void setPane(nw4hbm::lyt::Pane* pPane) { mpPane = pPane; }
    virtual nw4hbm::lyt::Pane* getPane() { return mpPane; }

    // members
  private:
    /* base Component */ // size 0x2c, offset 0x00
    nw4hbm::lyt::Pane* mpPane; // size 0x04, offset 0x2c
}; // size 0x30
} // namespace gui
} // namespace homebutton

#endif // RVL_SDK_HBM_HOMEBUTTON_GUI_MANAGER_HPP
