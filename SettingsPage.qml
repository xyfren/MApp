import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window // Нужно для получения разрешения монитора (Screen)
import androidtools
import mapp

Page {
    id: root

    signal exitRequested(string page)
    property string pageTitle: "Settings"

    ColumnLayout {
        anchors.centerIn: parent
        anchors.margins: 10
        spacing: 15

        RowLayout {
            Label {
                text: "Разрешение:"
                Layout.preferredWidth: 120
            }
            ComboBox {
                id: resolutionCombo
                Layout.fillWidth: true
                textRole: "text"

                model: [
                    {
                        text: "Разрешение монитора (" + AndroidTools.getDisplayParameters().width + "x" + AndroidTools.getDisplayParameters().height + ")",
                        w: AndroidTools.getDisplayParameters().width,
                        h: AndroidTools.getDisplayParameters().height
                    },
                    { text: "Full HD (1920x1080)", w: 1920, h: 1080 },
                    { text: "HD (1280x720)", w: 1280, h: 720 }
                ]

                // Устанавливаем корректный индекс при загрузке страницы
                Component.onCompleted: {
                    for (let i = 0; i < model.length; ++i) {
                        if (model[i].w === MAppSettings.width && model[i].h === MAppSettings.height) {
                            currentIndex = i
                            break
                        }
                    }
                }

                // При выборе пресета отправляем ширину и высоту в C++
                onActivated: {
                    let preset = model[currentIndex]
                    MAppSettings.width = preset.w
                    MAppSettings.height = preset.h
                }
            }
        }

        // --- Настройка качества (Slider) ---
        RowLayout {
            Label {
                text: "Качество:"
                Layout.preferredWidth: 120
            }
            Slider {
                id: qualitySlider
                from: 1
                to: 100
                stepSize: 1
                Layout.fillWidth: true

                // Предполагаем, что ты добавил поле quality в C++
                value: MAppSettings.quality
                onValueChanged: MAppSettings.quality = value
            }
            // Отображение текущего значения в процентах
            Label {
                text: qualitySlider.value + "%"
                Layout.preferredWidth: 40
                horizontalAlignment: Text.AlignRight
            }
        }

        // --- Настройка частоты обновления ---
        RowLayout {
            Label {
                text: "Частота кадров:"
                Layout.preferredWidth: 120
            }
            ComboBox {
                id: refreshRateCombo
                Layout.fillWidth: true
                textRole: "text"

                model: [
                    { text: "Частота монитора(" + AndroidTools.getDisplayParameters().refreshRate + ")",
                      refreshRate: AndroidTools.getDisplayParameters().refreshRate },
                    { text: "60", refreshRate: 60 },
                    { text: "30", refreshRate: 30 }
                ]

                // Устанавливаем корректный индекс при загрузке страницы
                Component.onCompleted: {
                    for (let i = 0; i < model.length; ++i) {
                        if (model[i].refreshRate === MAppSettings.refreshRate) {
                            currentIndex = i
                            break
                        }
                    }
                }

                // При выборе пресета отправляем ширину и высоту в C++
                onActivated: {
                    let preset = model[currentIndex]
                    MAppSettings.refreshRate = preset.refreshRate
                }
            }
        }

        // --- Выбор кодировщика ---
        RowLayout {
            Label {
                text: "Кодировщик:"
                Layout.preferredWidth: 120
            }
            ComboBox {
                textRole: "text"
                valueRole: "value"
                Layout.fillWidth: true
                model: [
                    { text: "Не выбрано", value: 0 },
                    { text: "FFmpeg", value: 1},
                    { text: "JPEG", value: 2}
                ]

                Component.onCompleted: currentIndex = indexOfValue(MAppSettings.coder)
                onActivated: MAppSettings.coder = currentValue
            }
        }
    }
    Shortcut {
        sequence: "Back"
        onActivated: exitRequested(pageTitle)
    }
}