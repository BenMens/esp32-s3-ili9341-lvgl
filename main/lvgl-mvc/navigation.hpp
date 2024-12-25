#pragma once

#include "view-controller.hpp"

class NavigationController
{
   public:
    virtual void pushViewController(ViewController &viewController) = 0;

    // virtual void replaceTopViewController(ViewController &viewController) =
    // 0;

    virtual void popViewController() = 0;
};

class DisplayViewControllerEntry
{
   public:
    ViewController &viewController;
    DisplayViewControllerEntry *previousEntry;

    DisplayViewControllerEntry(DisplayViewControllerEntry *previousEntry,
                               ViewController &viewController)
        : viewController(viewController), previousEntry(previousEntry)
    {
    }
};

enum class NavigationAction {
    PUSHED,
    POPPED,
};

class DisplayNavigationContoller : public NavigationController
{
   private:
    DisplayViewControllerEntry *topEntry = nullptr;
    lv_display_t *display = NULL;

   public:
    void loadScreen(DisplayViewControllerEntry &newEntry,
                    DisplayViewControllerEntry *prevEntry,
                    NavigationAction navAction)
    {
        if (prevEntry) {
            prevEntry->viewController.setWillAutoDelete();
        }

        if (navAction == NavigationAction::PUSHED) {
            lv_screen_load_anim(
                newEntry.viewController.attachViewToParent(NULL),
                LV_SCR_LOAD_ANIM_MOVE_LEFT, 100, 100, true);

        } else {
            lv_screen_load_anim(
                newEntry.viewController.attachViewToParent(NULL),
                LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 100, true);
        }
    }

    void setDisplay(lv_display_t *display)
    {
        this->display = display;
    }

    void pushViewController(ViewController &viewController)
    {
        topEntry = new DisplayViewControllerEntry(topEntry, viewController);
        DisplayViewControllerEntry *prevEntry = topEntry->previousEntry;

        viewController.setNavigationontroller(this);
        viewController.onPushed();

        if (prevEntry) {
            prevEntry->viewController.onChildPushed(&viewController);
        }

        lv_disp_set_default(display);
        loadScreen(*topEntry, prevEntry, NavigationAction::PUSHED);
    }

    void popViewController()
    {
        if (topEntry) {
            DisplayViewControllerEntry *prevTopEntry = topEntry;
            topEntry = prevTopEntry->previousEntry;

            prevTopEntry->viewController.onPopped();
            prevTopEntry->viewController.setNavigationontroller(nullptr);

            lv_disp_set_default(display);
            if (topEntry != nullptr) {
                loadScreen(*topEntry, prevTopEntry, NavigationAction::POPPED);
                topEntry->viewController.onChildPopped(
                    &prevTopEntry->viewController);
            } else {
                lv_screen_load(NULL);
            }

            delete prevTopEntry;
        }
    }
};
