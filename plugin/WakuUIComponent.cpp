#include "WakuUIComponent.h"
#include "WakuWidget.h"

QWidget* WakuUIComponent::createWidget(LogosAPI* logosAPI) {
    // LogosAPI parameter available but not used - WakuWidget creates its own
    return new WakuWidget();
}

void WakuUIComponent::destroyWidget(QWidget* widget) {
    delete widget;
}
