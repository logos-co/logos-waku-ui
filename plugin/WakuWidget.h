#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QListWidget>
#include <QTimer>
#include <string>
#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_sdk.h"

class WakuWidget : public QWidget {
    Q_OBJECT

public:
    explicit WakuWidget(QWidget* parent = nullptr);
    ~WakuWidget();
    
    // Waku operations
    void startWaku();
    void stopWaku();
    void refreshPeers();
    void refreshMetrics();

private slots:
    void onStartButtonClicked();
    void onStopButtonClicked();
    void onRefreshPeersClicked();
    void onRefreshMetricsClicked();
    void onMetricsTimer();

private:
    // UI elements
    QVBoxLayout* mainLayout;
    QHBoxLayout* buttonLayout;
    
    QLabel* statusLabel;
    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* refreshPeersButton;
    QPushButton* refreshMetricsButton;
    
    QLabel* peersLabel;
    QListWidget* peersList;
    
    QLabel* metricsLabel;
    QTextEdit* metricsDisplay;
    
    // LogosAPI instance for remote method calls
    LogosAPI* m_logosAPI;
    LogosModules* logos;
    
    // Connection status
    bool isWakuRunning;
    
    // Timer for auto-refreshing metrics
    QTimer* metricsTimer;
    
    // Helper methods
    void updateStatus(const QString& message);
    void updatePeersList(const QStringList& peers);
    void updateMetrics(const QString& metrics);
};
