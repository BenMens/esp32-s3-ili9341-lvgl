#pragma once

#include <lvgl.h>

#include "../lvgl-mvc/lvgl-mvc.hpp"

class NavigationController;

class ViewController
{
   private:
    ViewController *parentViewController = nullptr;
    NavigationController *navigationController = nullptr;
    lv_obj_t *view = NULL;
    lv_event_dsc_t *deleteEventDesc = NULL;
    bool willAutoDelete = false;

   protected:
    virtual lv_obj_t *createView(lv_obj_t *view) = 0;

   public:
    ViewController(ViewController *parentViewController)
        : parentViewController(parentViewController)
    {
    }

    virtual ~ViewController()
    {
        if (lvgl_mvc_lock(0)) {
            if (view && deleteEventDesc) {
                lv_obj_remove_event_dsc(view, deleteEventDesc);
            }
            deleteEventDesc = nullptr;
            deleteView();

            lvgl_mvc_unlock();
        }
    }

    virtual void deleteView()
    {
        if (view != nullptr) {
            if (lvgl_mvc_lock(0)) {
                if (!willAutoDelete) {
                    lv_obj_delete(view);
                }
                view = nullptr;
                deleteEventDesc = NULL;

                lvgl_mvc_unlock();
            }
        }
    }

    void setWillAutoDelete()
    {
        willAutoDelete = true;
    }

    bool viewValid()
    {
        return view != NULL;
    }

    virtual void update() {}

    virtual void onDidAppear() {}

    virtual void onWillDisappear() {}

    virtual void onChildPopped(ViewController *poppedViewController) {}

    virtual void onChildPushed(ViewController *pushedViewController) {}

    virtual lv_obj_t *getViewAttachedToParent(lv_obj_t *parent)
    {
        if (lvgl_mvc_lock(0)) {
            if (view == NULL) {
                view = createView(parent);

                deleteEventDesc = lv_obj_add_event_cb(
                    view,
                    [](lv_event_t *e) {
                        ViewController *viewController =
                            (ViewController *)lv_event_get_user_data(e);

                        viewController->view = NULL;
                        viewController->deleteEventDesc = NULL;
                    },
                    LV_EVENT_DELETE, this);

                update();
            } else if (lv_obj_get_parent(view) != parent) {
                lv_obj_set_parent(view, parent);
            }

            lvgl_mvc_unlock();

            return view;
        }

        return NULL;
    }

    void setNavigationontroller(NavigationController *navigationController)
    {
        this->navigationController = navigationController;
    }

    NavigationController *getNavigationontroller()
    {
        if (this->navigationController != nullptr) {
            return this->navigationController;
        } else if (this->parentViewController != nullptr) {
            return this->parentViewController->getNavigationontroller();
        } else {
            return nullptr;
        }
    }
};
