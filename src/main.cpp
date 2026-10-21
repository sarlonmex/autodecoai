#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <cocos2d.h>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace geode::prelude;

enum class StyleType {
    Light,
    Dark,
    Ocean,
    NeonGlow,
    CustomAI
};

enum class Language {
    German,
    English,
    Spanish,
    French,
    Asian
};

struct ModSettings {
    Language lang = Language::German;
    StyleType style = StyleType::CustomAI;
    int objectBudget = 1500;
    std::string aiPrompt = "Ocean Glow Style";
    bool tutorialCompleted = false;
};

static ModSettings g_settings;

// Mehrsprachigkeits-System (5 Sprachen)
std::string getTranslation(const std::string& key) {
    static const std::map<std::string, std::map<Language, std::string>> dict = {
        {"title", {
            {Language::German, "Auto Deco AI Menu"},
            {Language::English, "Auto Deco AI Menu"},
            {Language::Spanish, "Menú Auto Deco AI"},
            {Language::French, "Menu Auto Deco AI"},
            {Language::Asian, "Auto Deco AI 菜单"}
        }},
        {"generate", {
            {Language::German, "Level Dekorieren"},
            {Language::English, "Decorate Level"},
            {Language::Spanish, "Decorar Nivel"},
            {Language::French, "Décorer le Niveau"},
            {Language::Asian, "装饰关卡"}
        }},
        {"tutorial_btn", {
            {Language::German, "Tutorial"},
            {Language::English, "Tutorial"},
            {Language::Spanish, "Tutorial"},
            {Language::French, "Tutoriel"},
            {Language::Asian, "教程"}
        }},
        {"skip_tut", {
            {Language::German, "Tutorial Überspringen"},
            {Language::English, "Skip Tutorial"},
            {Language::Spanish, "Omitir Tutorial"},
            {Language::French, "Passer le Tutoriel"},
            {Language::Asian, "跳过教程"}
        }},
        {"tut_text", {
            {Language::German, "Willkommen bei autodecoai!\n1. Gib dein Wunsch-Thema ein.\n2. Wähle die Objekt-Anzahl.\n3. Klicke auf 'Level Dekorieren'."},
            {Language::English, "Welcome to autodecoai!\n1. Enter your prompt.\n2. Choose object budget.\n3. Click 'Decorate Level'."},
            {Language::Spanish, "¡Bienvenido a autodecoai!\n1. Ingresa tu estilo.\n2. Elige la cantidad de objetos.\n3. Haz clic en 'Decorar Nivel'."},
            {Language::French, "Bienvenue sur autodecoai!\n1. Entrez votre style.\n2. Choisissez le nombre d'objets.\n3. Cliquez sur 'Décorer'."},
            {Language::Asian, "欢迎使用 autodecoai！\n1. 输入你想要的风格。\n2. 选择物体数量。\n3. 点击“装饰关卡”。"}
        }}
    };

    if (dict.count(key) && dict.at(key).count(g_settings.lang)) {
        return dict.at(key).at(g_settings.lang);
    }
    return key;
}

// Auto-Deco Logik Generator
class AutoDecoEngine {
public:
    static void processLevel(LevelEditorLayer* editor) {
        if (!editor) return;

        auto objects = editor->m_objects;
        if (!objects) return;

        int addedObjects = 0;
        int targetBudget = g_settings.objectBudget;

        std::string promptLower = g_settings.aiPrompt;
        std::transform(promptLower.begin(), promptLower.end(), promptLower.begin(), ::tolower);

        ccColor3B mainColor = ccc3(255, 255, 255);
        ccColor3B glowColor = ccc3(0, 255, 255);

        if (promptLower.find("ocean") != std::string::npos || promptLower.find("meer") != std::string::npos) {
            mainColor = ccc3(10, 50, 130);
            glowColor = ccc3(0, 220, 255);
        } else if (promptLower.find("dark") != std::string::npos || promptLower.find("dunkel") != std::string::npos) {
            mainColor = ccc3(20, 20, 25);
            glowColor = ccc3(180, 0, 255);
        } else if (promptLower.find("hell") != std::string::npos || promptLower.find("light") != std::string::npos) {
            mainColor = ccc3(240, 240, 200);
            glowColor = ccc3(255, 200, 50);
        }

        for (unsigned int i = 0; i < objects->count(); ++i) {
            if (addedObjects >= targetBudget) break;

            auto obj = static_cast<GameObject*>(objects->objectAtIndex(i));
            if (!obj) continue;

            if (obj->m_objectID < 1) continue;

            auto glowObj = editor->createObject(211, obj->getPosition(), true);
            if (glowObj) {
                glowObj->setRgbColor(glowColor);
                glowObj->setScale(obj->getScale());
                addedObjects++;
            }
        }

        Notification::create("Level erfolgreich dekoriert!", NotificationIcon::Success)->show();
    }
};

// GUI Pop-up Menu
class AutoDecoPopup : public Popup<std::string const&> {
protected:
    bool setup(std::string const& value) override {
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        this->setTitle(getTranslation("title"));

        auto langBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("Lang", "goldFont.fnt", "GJ_button_01.png", 0.8f),
            this,
            menu_selector(AutoDecoPopup::onToggleLanguage)
        );
        
        auto menu = CCMenu::create();
        menu->addChild(langBtn);
        langBtn->setPosition({ -130.f, 80.f });

        auto decoBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(getTranslation("generate").c_str(), "goldFont.fnt", "GJ_button_02.png", 0.8f),
            this,
            menu_selector(AutoDecoPopup::onDecorate)
        );
        menu->addChild(decoBtn);
        decoBtn->setPosition({ 0.f, -70.f });

        auto tutBtn = CCMenuItemSpriteExtra::create(
            ButtonSprite::create(getTranslation("tutorial_btn").c_str(), "goldFont.fnt", "GJ_button_04.png", 0.7f),
            this,
            menu_selector(AutoDecoPopup::onTutorial)
        );
        menu->addChild(tutBtn);
        tutBtn->setPosition({ 120.f, 80.f });

        m_mainLayer->addChild(menu);

        auto infoLabel = CCLabelBMFont::create("Style: KI (Auto Glow / Ocean / Dark / Light)", "bigFont.fnt");
        infoLabel->setScale(0.4f);
        infoLabel->setPosition({ winSize.width / 2, winSize.height / 2 + 20.f });
        m_mainLayer->addChild(infoLabel);

        return true;
    }

    void onToggleLanguage(CCObject*) {
        int nextLang = (static_cast<int>(g_settings.lang) + 1) % 5;
        g_settings.lang = static_cast<Language>(nextLang);
        Notification::create("Language switched!", NotificationIcon::Info)->show();
    }

    void onTutorial(CCObject*) {
        FLAlertLayer::create(
            getTranslation("tutorial_btn").c_str(),
            getTranslation("tut_text"),
            getTranslation("skip_tut")
        )->show();
    }

    void onDecorate(CCObject*) {
        auto editor = LevelEditorLayer::get();
        if (editor) {
            AutoDecoEngine::processLevel(editor);
            this->onClose(nullptr);
        } else {
            FLAlertLayer::create("Error", "Öffne das Mod-Menü innerhalb des Level Editors!", "OK")->show();
        }
    }

public:
    static AutoDecoPopup* create() {
        auto ret = new AutoDecoPopup();
        if (ret && ret->initAnchored(360.f, 240.f, "autodecoai")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};

// Hook in den EditorUI
class $modify(MyEditorUI, EditorUI) {
    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor)) return false;

        auto sprite = ButtonSprite::create("AI", "goldFont.fnt", "GJ_button_01.png", 0.8f);
        auto btn = CCMenuItemSpriteExtra::create(
            sprite,
            this,
            menu_selector(MyEditorUI::onAutoDecoMenu)
        );

        auto menu = this->getChildByID("play-menu");
        if (menu) {
            menu->addChild(btn);
            menu->updateLayout();
        } else {
            this->addChild(btn);
            btn->setPosition({ 40.f, 200.f });
        }

        return true;
    }

    void onAutoDecoMenu(CCObject*) {
        AutoDecoPopup::create()->show();
    }
};
