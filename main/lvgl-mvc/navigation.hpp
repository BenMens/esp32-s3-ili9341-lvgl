#pragma once

#include "view-controller.hpp"

enum class NavigationAction {
    PUSH,
    POP,
    CAROUSEL_LEFT,
    CAROUSEL_RIGHT,
};


class NavigationController : public ViewController
{
   public:
    NavigationController() : ViewController(NULL) {}

    virtual void pushViewController(ViewController &viewController) = 0;

    // virtual void replaceTopViewController(ViewController &viewController) =
    // 0;

    virtual void popViewController() = 0;

    void triggerNavigation(NavigationAction action)
    {
        switch (action) {
            case NavigationAction::POP:
                popViewController();
                break;
            default:
                // Invalid action
                break;
        }
    }
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

        if (navAction == NavigationAction::PUSH) {
            lv_screen_load_anim(
                newEntry.viewController.getViewAttachedToParent(NULL),
                LV_SCR_LOAD_ANIM_FADE_IN, 200, 0, true);
        } else {
            lv_screen_load_anim(
                newEntry.viewController.getViewAttachedToParent(NULL),
                LV_SCR_LOAD_ANIM_FADE_OUT, 200, 0, true);
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
        loadScreen(*topEntry, prevEntry, NavigationAction::PUSH);
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
                loadScreen(*topEntry, prevTopEntry, NavigationAction::POP);
                topEntry->viewController.onChildPopped(
                    &prevTopEntry->viewController);
            } else {
                lv_screen_load(NULL);
            }

            delete prevTopEntry;
        }
    }

    lv_obj_t *createView(lv_obj_t *parent)
    {
        if (topEntry != NULL) {
            return topEntry->viewController.getViewAttachedToParent(parent);
        } else {
            return NULL;
        }
    }
};
