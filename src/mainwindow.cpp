/*******************************************************************************

Copyright (C) The University of Auckland

OpenCOR is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OpenCOR is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************/

//==============================================================================
// Main window
//==============================================================================

#include "centralwidget.h"
#include "checkforupdatesdialog.h"
#include "cliutils.h"
#include "coreinterface.h"
#include "guiapplication.h"
#include "guiinterface.h"
#include "guiutils.h"
#include "i18ninterface.h"
#include "mainwindow.h"
#include "plugininterface.h"
#include "pluginmanager.h"
#include "pluginsdialog.h"
#include "preferencesdialog.h"
#include "preferencesinterface.h"
#include "viewinterface.h"
#include "windowinterface.h"
#include "windowwidget.h"

//==============================================================================

#include "ui_mainwindow.h"

//==============================================================================

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

//==============================================================================

#include <QAction>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QLocale>
#include <QMenu>
#include <QMenuBar>
#include <QRect>
#include <QSettings>
#include <QShortcut>
#include <QUrl>
#include <QWindow>

//==============================================================================

#ifdef Q_OS_WIN
    #include <shlobj.h>
#endif

//==============================================================================

#ifdef Q_OS_MAC
    #include <CoreServices/CoreServices.h>
#endif

//==============================================================================

namespace OpenCOR {

//==============================================================================

static const auto EnglishLocale = QStringLiteral("en");
static const auto FrenchLocale  = QStringLiteral("fr");

//==============================================================================

MainWindow::MainWindow(const QString &pPluginsDir,
                       const QString &pApplicationDate) :
    QMainWindow(),
    mGui(new Ui::MainWindow),
    mApplicationDate(pApplicationDate),
    mLoadedPluginPlugins(Plugins()),
    mLoadedI18nPlugins(Plugins()),
    mLoadedGuiPlugins(Plugins()),
    mLoadedPreferencesPlugins(Plugins()),
    mLoadedWindowPlugins(Plugins()),
    mCoreInterface(0),
    mRawLocale(QString()),
    mMenus(QMap<QString, QMenu *>()),
    mFileNewMenu(0),
    mViewWindowsMenu(0),
    mViewSeparator(0),
    mViewPlugin(0),
    mDockedWindowsVisible(true),
    mDockedWindowsState(QByteArray())
{
    // Make sure that OpenCOR can handle a file opening request (from the
    // operating system), as well as a message sent by another instance of
    // itself

    GuiApplication *guiApplication = qobject_cast<GuiApplication *>(qApp);

    connect(guiApplication, &GuiApplication::fileOpenRequest,
            this, &MainWindow::openFileOrHandleUrl);
    connect(guiApplication, &GuiApplication::messageReceived,
            this, &MainWindow::handleMessage);

    // Handle OpenCOR URLs
    // Note: we should, through our GuiApplication class (see main.cpp), be able
    //       to handle OpenCOR URLs (not least because we make sure that the
    //       OpenCOR URL scheme is set; see the call to
    //       registerOpencorUrlScheme() below), but our URL handler ensures that
    //       it will work whether the OpenCOR URL scheme is set or not (in case
    //       it can't be set on a given platform)...

    QDesktopServices::setUrlHandler("opencor", this, "handleUrl");

    // Register our OpenCOR URL scheme

    registerOpencorUrlScheme();

    // Create our settings object

    mSettings = new QSettings();

    // Create our plugin manager (which will automatically load our various
    // plugins)

    mPluginManager = new PluginManager(pPluginsDir);

    // Retrieve some categories of plugins

    foreach (Plugin *plugin, mPluginManager->loadedPlugins()) {
        if (qobject_cast<PluginInterface *>(plugin->instance()))
            mLoadedPluginPlugins << plugin;

        if (qobject_cast<I18nInterface *>(plugin->instance()))
            mLoadedI18nPlugins << plugin;

        if (qobject_cast<GuiInterface *>(plugin->instance()))
            mLoadedGuiPlugins << plugin;

        if (qobject_cast<PreferencesInterface *>(plugin->instance()))
            mLoadedPreferencesPlugins << plugin;

        if (qobject_cast<WindowInterface *>(plugin->instance()))
            mLoadedWindowPlugins << plugin;
    }

    // Retrieve our Core plugin's interface, should the Core plugin be loaded

    if (mPluginManager->corePlugin())
        mCoreInterface = qobject_cast<CoreInterface *>(mPluginManager->corePlugin()->instance());

    // Set up the GUI

    mGui->setupUi(this);

    connect(mGui->actionFullScreen, &QAction::triggered,
            this, &MainWindow::actionFullScreenTriggered);
    connect(mGui->actionSystem, &QAction::triggered,
            this, &MainWindow::actionSystemTriggered);
    connect(mGui->actionEnglish, &QAction::triggered,
            this, &MainWindow::actionEnglishTriggered);
    connect(mGui->actionFrench, &QAction::triggered,
            this, &MainWindow::actionFrenchTriggered);
    connect(mGui->actionPlugins, &QAction::triggered,
            this, &MainWindow::actionPluginsTriggered);
    connect(mGui->actionPreferences, &QAction::triggered,
            this, &MainWindow::actionPreferencesTriggered);
    connect(mGui->actionHomePage, &QAction::triggered,
            this, &MainWindow::actionHomePageTriggered);
    connect(mGui->actionCheckForUpdates, &QAction::triggered,
            this, &MainWindow::actionCheckForUpdatesTriggered);
    connect(mGui->actionAbout, &QAction::triggered,
            this, &MainWindow::actionAboutTriggered);

    // Set the role of some of our menu items, so that macOS can move them into
    // the application menu

    mGui->actionQuit->setMenuRole(QAction::QuitRole);
    mGui->actionPreferences->setMenuRole(QAction::PreferencesRole);
    mGui->actionAbout->setMenuRole(QAction::AboutRole);
    mGui->actionCheckForUpdates->setMenuRole(QAction::ApplicationSpecificRole);

    // Title of our main window
    // Note: we don't set it in our .ui file since this will require
    //       'translating' it in other languages...

    setWindowTitle(qAppName());

    // Customise our docked windows action and handle it through a connection
    // Note #1: the reason for having several shortcuts is because one or
    //          several of them are bound to be already used on the target
    //          system...
    // Note #2: normally we would call setShortcuts() rather than setShortcut()
    //          and then manually creating several QShortcut objects, but it
    //          doesn't work (bug?)...

    mGui->actionDockedWindows->setShortcut(QKeySequence(Qt::ControlModifier|Qt::Key_Space));

    connect(mGui->actionDockedWindows, &QAction::triggered,
            this, QOverload<bool>::of(&MainWindow::showDockedWindows));

    new QShortcut(QKeySequence(Qt::MetaModifier|Qt::Key_Space),
                  this, SLOT(toggleDockedWindows()));
    new QShortcut(QKeySequence(Qt::AltModifier|Qt::Key_Space),
                  this, SLOT(toggleDockedWindows()));
    new QShortcut(QKeySequence(Qt::ControlModifier|Qt::MetaModifier|Qt::Key_Space),
                  this, SLOT(toggleDockedWindows()));
    new QShortcut(QKeySequence(Qt::ControlModifier|Qt::AltModifier|Qt::Key_Space),
                  this, SLOT(toggleDockedWindows()));
    new QShortcut(QKeySequence(Qt::MetaModifier|Qt::AltModifier|Qt::Key_Space),
                  this, SLOT(toggleDockedWindows()));
    new QShortcut(QKeySequence(Qt::ControlModifier|Qt::MetaModifier|Qt::AltModifier|Qt::Key_Space),
                  this, SLOT(toggleDockedWindows()));

    // A connection to handle the status bar

    connect(mGui->actionStatusBar, &QAction::toggled,
            mGui->statusBar, &QStatusBar::setVisible);

    // Some connections to handle our various menu items

    connect(mGui->actionQuit, &QAction::triggered,
            this, &MainWindow::close);
    connect(mGui->actionResetAll, &QAction::triggered,
            this, &MainWindow::resetAll);

    // Set the shortcuts of some actions
    // Note: we do it here, so that we can use standard shortcuts (whenever
    //       possible)...

#if defined(Q_OS_WIN) || defined(Q_OS_LINUX)
    // Note: normally, we would be using QKeySequence::Quit, but we want to
    //       support both Alt+F4 and Ctrl+Q on Windows and Linux, and the
    //       default key sequence doesn't, so we set them ourselves...

    mGui->actionQuit->setShortcuts(QList<QKeySequence>()
                                       << QKeySequence(Qt::AltModifier|Qt::Key_F4)
                                       << QKeySequence(Qt::ControlModifier|Qt::Key_Q));
#elif defined(Q_OS_MAC)
    mGui->actionQuit->setShortcut(QKeySequence::Quit);
#else
    #error Unsupported platform
#endif

#ifdef Q_OS_MAC
    // A special shortcut to have OpenCOR minimised on macOS when pressing Cmd+M
    // Note: indeed, when pressing Cmd+M on macOS, the active application is
    //       expected to minimise itself, but it doesn't using Qt only...

    new QShortcut(QKeySequence(Qt::ControlModifier|Qt::Key_M),
                  this, SLOT(showMinimized()));
#endif

    mGui->actionFullScreen->setShortcut(QKeySequence::FullScreen);

    // Initialise the plugins themselves

    foreach (Plugin *plugin, mLoadedPluginPlugins)
        qobject_cast<PluginInterface *>(plugin->instance())->initializePlugin();

    // Initialise the plugin further by doing things that can only be done by
    // OpenCOR itself (e.g. set the central widget, create some menus)

    foreach (Plugin *plugin, mPluginManager->loadedPlugins())
        initializeGuiPlugin(plugin);

    // Let our various plugins know that all of them have been initialised
    // Note: this is important to do since the initialisation of a plugin is
    //       something that is done without knowing anything about other
    //       plugins. However, there may be things that require knowledge of
    //       what one or several other plugins are about, and this is something
    //       that can only be done once all the plugins have been initialised
    //       (e.g. the SimulationExperimentView plugin needs to know which
    //       solvers, if any, are available to it)...

    foreach (Plugin *plugin, mLoadedPluginPlugins)
        qobject_cast<PluginInterface *>(plugin->instance())->pluginsInitialized(mPluginManager->loadedPlugins());

    // Keep track of the showing/hiding of the different window widgets

    foreach (Plugin *plugin, mLoadedWindowPlugins) {
        connect(qobject_cast<WindowInterface *>(plugin->instance())->windowWidget(), &QDockWidget::visibilityChanged,
                this, &MainWindow::updateDockWidgetsVisibility);
    }

    // Show/hide and enable/disable the windows action depending on whether
    // there are window widgets

    showEnableAction(mGui->actionDockedWindows, mLoadedWindowPlugins.count());

    // Retrieve the user settings from the previous session, if any

    loadSettings();

    // Initialise the checked state of our full screen action, since OpenCOR may
    // (re)start in full screen mode

    mGui->actionFullScreen->setChecked(isFullScreen());

    // We are done initialising ourselves, so open/handle any file / OpenCOR URL
    // we have been tracking until now
    // Note: the way we open/handle those files / OpenCOR URLs ensures that we
    //       can still receive files / OpenCOR URLs to open/handle while we
    //       start opening/handling those that we have in stock, and this in the
    //       correct order...

    while (guiApplication->hasFileNamesOrOpencorUrls())
        openFileOrHandleUrl(guiApplication->firstFileNameOrOpencorUrl());

    guiApplication->updateCanEmitFileOpenRequestSignal();
}

//==============================================================================

MainWindow::~MainWindow()
{
    // Stop tracking the showing/hiding of the different window widgets
    // Note: indeed, to call updateDockWidgetsVisibility() when shutting down
    //       (e.g. as a result of selecting Tools | Reset All) doesn't make
    //       sense and will, in fact, crash OpenCOR...

    foreach (Plugin *plugin, mLoadedWindowPlugins) {
        disconnect(qobject_cast<WindowInterface *>(plugin->instance())->windowWidget(), &QDockWidget::visibilityChanged,
                   this, &MainWindow::updateDockWidgetsVisibility);
    }

    // Finalise our plugins
    // Note: we do this in reverse to ensure that dependent objects are deleted
    //       in the correct order...

    for (auto plugin = mLoadedPluginPlugins.rbegin(), pluginEnd = mLoadedPluginPlugins.rend();
         plugin != pluginEnd; ++plugin) {
        qobject_cast<PluginInterface *>((*plugin)->instance())->finalizePlugin();
    }

    // Delete our central widget
    // Note: if we don't have one, then nothing will happen...

    delete centralWidget();

    // Delete some internal objects

    delete mPluginManager;
    delete mSettings;

    // Delete the GUI

    delete mGui;
}

//==============================================================================

void MainWindow::changeEvent(QEvent *pEvent)
{
    // Default handling of the event

    QMainWindow::changeEvent(pEvent);

    // Do a few more things for some changes

    if (   (pEvent->type() == QEvent::LocaleChange)
        && (mGui->actionSystem->isChecked())) {
        // The system's locale has changed, so update OpenCOR's locale in case
        // the user wants to use the system's locale

        setLocale();
#ifdef Q_OS_MAC
    } else if (pEvent->type() == QEvent::WindowStateChange) {
        // The window state has changed, so update the checked state of our full
        // screen action
        // Note: useful on macOS since there is a special full screen button in
        //       the main window's title bar...

        mGui->actionFullScreen->setChecked(isFullScreen());
#endif
    }
}

//==============================================================================

void MainWindow::closeEvent(QCloseEvent *pEvent)
{
    // Check with our Core plugin, if it has been loaded, that it's OK to close

    bool canClose = true;

    if (mCoreInterface)
        canClose = mCoreInterface->canClose();

    // Close ourselves, if possible

    if (canClose) {
        // Delete any Web inspector window (which may have been created through
        // our use of QtWebKit)

        foreach (QWindow *window, QGuiApplication::topLevelWindows()) {
            if (!window->objectName().compare("QWebInspectorClassWindow"))
                window->close();
        }

        // Keep track of our default settings
        // Note: it must be done here, as opposed to the destructor, otherwise
        //       some settings (e.g. docked windows) won't be properly saved...

        saveSettings();

        // Accept the event

        pEvent->accept();
    } else {
        // Ignore the event, i.e. don't close ourselves

        pEvent->ignore();
    }
}

//==============================================================================

#ifdef Q_OS_MAC
void MainWindow::keyPressEvent(QKeyEvent *pEvent)
{
    // Exit full-screen mode on macOS when pressing Esc
    // Note #1: indeed, when pressing Esc on macOS, the active application is
    //          expected to exit full-screen mode, but it doesn't using Qt
    //          only...
    // Note #2: normally, we would do this through a shortcut (as we are doing
    //          with Cmd+M to minimise OpenCOR on macOS, but then we wouldn't be
    //          able to close the find/replace widget of a text editor...

    if (   !(pEvent->modifiers() & Qt::ShiftModifier)
        && !(pEvent->modifiers() & Qt::ControlModifier)
        && !(pEvent->modifiers() & Qt::AltModifier)
        && !(pEvent->modifiers() & Qt::MetaModifier)
        &&  (pEvent->key() == Qt::Key_Escape)) {
        if (isFullScreen())
            showNormal();

        pEvent->accept();
    } else {
        // Default handling of the event

        QMainWindow::keyPressEvent(pEvent);
    }
}
#endif

//==============================================================================

void MainWindow::registerOpencorUrlScheme()
{
    // Register our OpenCOR URL scheme

#if defined(Q_OS_WIN)
    QSettings settings("HKEY_CURRENT_USER\\Software\\Classes", QSettings::NativeFormat);
    QString applicationFileName = QDir::toNativeSeparators(canonicalFileName(qApp->applicationFilePath()));

    settings.setValue("opencor/Default", QString("URL:%1 link").arg(qApp->applicationName()));
    settings.setValue("opencor/Content Type", "x-scheme-handler/opencor");
    settings.setValue("opencor/URL Protocol", "");
    settings.setValue("opencor/DefaultIcon/Default", "\""+applicationFileName+"\",1");
    settings.setValue("opencor/shell/Default", "open");
    settings.setValue("opencor/shell/open/command/Default", "\""+applicationFileName+"\" \"%1\"");

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
#elif defined(Q_OS_LINUX)
    if (!exec("which", QStringList() << "xdg-mime")) {
        QString iconPath = canonicalFileName(QString("%1/.local/share/%2/%3/%3.png").arg(QDir::homePath())
                                                                                    .arg(qApp->organizationName())
                                                                                    .arg(qApp->applicationName()));

        writeResourceToFile(iconPath, ":/app_icon");

        writeFileContentsToFile(QString("%1/.local/share/applications/opencor.desktop").arg(QDir::homePath()),
                                QString("[Desktop Entry]\n"
                                        "Type=Application\n"
                                        "Name=%1\n"
                                        "Exec=%2 %u\n"
                                        "Icon=%3\n"
                                        "Terminal=false\n"
                                        "MimeType=x-scheme-handler/opencor\n").arg(qApp->applicationName())
                                                                              .arg(canonicalFileName(qApp->applicationFilePath()))
                                                                              .arg(iconPath));

        exec("xdg-mime", QStringList() << "default" << "opencor.desktop" << "x-scheme-handler/opencor");
    }
#elif defined(Q_OS_MAC)
    LSSetDefaultHandlerForURLScheme(CFSTR("opencor"),
                                    CFBundleGetIdentifier(CFBundleGetMainBundle()));
#else
    #error Unsupported platform
#endif
}

//==============================================================================

void MainWindow::initializeGuiPlugin(Plugin *pPlugin)
{
    // Retrieve and apply the plugin's GUI settings, if any

    GuiInterface *guiInterface = qobject_cast<GuiInterface *>(pPlugin->instance());

    if (guiInterface) {
        // Add the menus before the View menu or merge them to an existing menu,
        // if needed
        // Note: we must do that in reverse order since we are inserting menus,
        //       as opposed to appending some...

        Gui::Menus guiMenus = guiInterface->guiMenus();

        for (int i = guiMenus.count()-1; i >= 0; --i) {
            if (guiMenus[i].type() == Gui::Menu::View) {
                QMenu *newMenu = guiMenus[i].menu();
                QString newMenuName = newMenu->objectName();

                QMenu *oldMenu = mMenus.value(newMenuName);

                if (oldMenu && !guiMenus[i].action()) {
                    // A menu with the same name already exists, so add the
                    // contents of our new menu to the existing one

                    oldMenu->addSeparator();
                    oldMenu->addActions(newMenu->actions());

                    // Delete our new menu since we don't need it anymore

                    delete newMenu;
                } else {
                    // No menu with the same name already exists (or our menu
                    // doesn't have a name), so add our new menu to our menu bar

                    mGui->menuBar->insertAction(mGui->menuView->menuAction(),
                                                newMenu->menuAction());

                    // Keep track of our new menu, but only if it has a name

                    if (!newMenuName.isEmpty())
                        mMenus.insert(newMenuName, newMenu);
                }
            }
        }

        // Add the actions/separators to our File or Tools menu
        // Note: as for the menus above, we must do it in reverse order since we
        //       are inserting actions, as opposed to appending some...

        Gui::MenuActions guiMenuActions = guiInterface->guiMenuActions();

        for (int i = guiMenuActions.count()-1; i >= 0; --i) {
            // Insert the action/separator to the right menu, if any

            QMenu *menu = 0;

            if (guiMenuActions[i].type() == Gui::MenuAction::File)
                menu = mGui->menuFile;
            else if (guiMenuActions[i].type() == Gui::MenuAction::Tools)
                menu = mGui->menuTools;

            if (menu) {
                menu->insertAction((menu == mGui->menuTools)?
                                       menu->actions()[2]:
                                       menu->actions().first(),
                                   guiMenuActions[i].action());
            }
        }

        // Add some sub-menus before some menu item/separator in our File menu

        foreach (const Gui::Menu &guiMenu, guiMenus) {
            if (guiMenu.action() && (guiMenu.type() == Gui::Menu::File))
                mGui->menuFile->insertMenu(guiMenu.action(), guiMenu.menu());
        }

        // Add some actions to our File|New menu and keep track of them

        static QString pluginForFileNewMenu = QString();

        foreach (const Gui::MenuAction &guiMenuAction, guiMenuActions) {
            if (guiMenuAction.type() == Gui::MenuAction::FileNew) {
                // Check whether the File|New menu has been created and if not,
                // then create it

                if (!mFileNewMenu) {
                    // The menu doesn't already exist, so create it

                    mFileNewMenu = new QMenu(this);

                    mFileNewMenu->menuAction()->setIcon(QIcon(":/oxygen/mimetypes/application-x-zerosize.png"));

                    // Add the New menu to our File menu and add a separator
                    // after it

                    mGui->menuFile->insertMenu(mGui->menuFile->actions().first(),
                                               mFileNewMenu);
                    mGui->menuFile->insertSeparator(mGui->menuFile->actions()[1]);

                    pluginForFileNewMenu = pPlugin->name();
                } else if (pluginForFileNewMenu.compare(pPlugin->name())) {
                    // The File|New menu already exists, so add a separator to
                    // it so that previous menu items (from a different plugin)
                    // don't get mixed up with the new one

                    mFileNewMenu->addSeparator();

                    pluginForFileNewMenu = pPlugin->name();
                }

                mFileNewMenu->addAction(guiMenuAction.action());
            }
        }
    }

    // Set our central widget, in case we are dealing with the Core plugin

    if (pPlugin == mPluginManager->corePlugin()) {
        // We are dealing with the Core plugin, so set our central widget

        setCentralWidget(qobject_cast<CoreInterface *>(pPlugin->instance())->centralWidget());
        // Note: if the Core plugin is loaded, then it means it supports the
        //       Core interface, so no need to check anything...

        // Also keep track of GUI updates in our central widget
        // Note: we cannot use the new connect() syntax since the signal is
        //       located in our Core plugin and that we don't know anything
        //       about it...

        connect(static_cast<Core::CentralWidget *>(centralWidget()), SIGNAL(guiUpdated(OpenCOR::Plugin *, const QString &)),
                this, SLOT(updateGui(OpenCOR::Plugin *, const QString &)));
    }

    // Add the plugin's window, in case we are dealing with a window plugin

    WindowInterface *windowInterface = qobject_cast<WindowInterface *>(pPlugin->instance());

    if (windowInterface) {
        // Dock the window to its default dock area

        addDockWidget(windowInterface->windowDefaultDockArea(), windowInterface->windowWidget());

        // Add an action to our menu to show/hide the window

        if (!pPlugin->name().compare("HelpWindow")) {
            // Special case of the help window

            mGui->menuHelp->insertAction(mGui->actionHomePage,
                                         windowInterface->windowAction());
            mGui->menuHelp->insertSeparator(mGui->actionHomePage);
        } else {
            // Update the View menu by adding the action to the
            // View|Windows menu

            updateViewWindowsMenu(windowInterface->windowAction());
        }

        // Connect the action to the window

        connect(windowInterface->windowAction(), &QAction::triggered,
                windowInterface->windowWidget(), &QDockWidget::setVisible);
        connect(windowInterface->windowWidget()->toggleViewAction(), &QAction::toggled,
                windowInterface->windowAction(), &QAction::setChecked);
    }
}

//==============================================================================

static const auto SettingsGeometry             = QStringLiteral("Geometry");
static const auto SettingsState                = QStringLiteral("State");
static const auto SettingsDockedWindowsVisible = QStringLiteral("DockedWindowsVisible");
static const auto SettingsDockedWindowsState   = QStringLiteral("DockedWindowsState");
static const auto SettingsStatusBarVisible     = QStringLiteral("StatusBarVisible");

//==============================================================================

void MainWindow::loadSettings()
{
    // Retrieve and set the language to be used by OpenCOR
    // Note: the setting is forced in order to account for locale-dependent
    //       initialisations (e.g. see CentralWidget::retranslateUi())...

    setLocale(rawLocale(), true);

    // Retrieve the geometry and state of the main window

    if (   !restoreGeometry(mSettings->value(SettingsGeometry).toByteArray())
        || !restoreState(mSettings->value(SettingsState).toByteArray())) {
        // The geometry and/or state of the main window couldn't be retrieved,
        // so go with some default settings

        // Default size and position of the main window

        QRect desktopGeometry = qApp->desktop()->availableGeometry();
        int horizSpace = desktopGeometry.width()/13;
        int vertSpace = desktopGeometry.height()/13;

        setGeometry(desktopGeometry.left()+horizSpace,
                    desktopGeometry.top()+vertSpace,
                    desktopGeometry.width()-2*horizSpace,
                    desktopGeometry.height()-2*vertSpace);
    }

    // Retrieve whether the docked windows are to be shown

    showDockedWindows(mSettings->value(SettingsDockedWindowsVisible, true).toBool(), true);

    // Retrieve the state of the docked windows

    mDockedWindowsState = mSettings->value(SettingsDockedWindowsState, QByteArray()).toByteArray();

    // Retrieve the settings of our various plugins

    foreach (Plugin *plugin, mLoadedPluginPlugins) {
        mSettings->beginGroup(SettingsPlugins);
            mSettings->beginGroup(plugin->name());
                qobject_cast<PluginInterface *>(plugin->instance())->loadSettings(mSettings);
            mSettings->endGroup();
        mSettings->endGroup();
    }

    // Let our core plugin know that all of the plugins have loaded their
    // settings
    // Note: this is similar to initializePlugin() vs. pluginsInitialized()...

    if (mCoreInterface)
        mCoreInterface->settingsLoaded(mPluginManager->loadedPlugins());

    // Remove the File menu when on macOS, should no plugins be loaded
    // Note: our File menu should only contain the Exit menu item, but on macOS
    //       that menu item gets automatically moved to the application menu...

#ifdef Q_OS_MAC
    mGui->menuFile->menuAction()->setVisible(mPluginManager->loadedPlugins().count());
#endif

    // Retrieve whether the status bar is to be shown

    mGui->actionStatusBar->setChecked(mSettings->value(SettingsStatusBarVisible, true).toBool());
}

//==============================================================================

void MainWindow::saveSettings() const
{
    // Keep track of the geometry and state of the main window

    mSettings->setValue(SettingsGeometry, saveGeometry());
    mSettings->setValue(SettingsState, saveState());

    // Keep track of whether the docked windows are to be shown

    mSettings->setValue(SettingsDockedWindowsVisible, mDockedWindowsVisible);

    // Keep track of the state of the docked windows

    mSettings->setValue(SettingsDockedWindowsState, mDockedWindowsState);

    // Keep track of whether the status bar is to be shown

    mSettings->setValue(SettingsStatusBarVisible,
                        mGui->statusBar->isVisible());

    // Keep track of the settings of our various plugins

    foreach (Plugin *plugin, mLoadedPluginPlugins) {
        mSettings->beginGroup(SettingsPlugins);
            mSettings->beginGroup(plugin->name());
                qobject_cast<PluginInterface *>(plugin->instance())->saveSettings(mSettings);
            mSettings->endGroup();
        mSettings->endGroup();
    }
}

//==============================================================================

void MainWindow::setLocale(const QString &pRawLocale, bool pForceSetting)
{
    QString systemLocale = QLocale::system().name().left(2);

    QString oldLocale = mRawLocale.isEmpty()?systemLocale:mRawLocale;
    QString newLocale = pRawLocale.isEmpty()?systemLocale:pRawLocale;

    // Keep track of the new locale, if needed

    if (pRawLocale.compare(mRawLocale) || pForceSetting) {
        mRawLocale = pRawLocale;

        // Also keep a copy of the new raw locale in our settings (so that the
        // new locale can be retrieved from plugins)

        setRawLocale(mRawLocale);
    }

    // Check whether the new locale is different from the old one and if so,
    // then retranslate everything

    if (oldLocale.compare(newLocale) || pForceSetting) {
        // Specify the language to be used by OpenCOR

        QLocale::setDefault(QLocale(newLocale));

        qApp->removeTranslator(&mQtBaseTranslator);
        mQtBaseTranslator.load(QString(":/translations/qtbase_%1.qm").arg(newLocale));
        qApp->installTranslator(&mQtBaseTranslator);

        qApp->removeTranslator(&mQtHelpTranslator);
        mQtHelpTranslator.load(QString(":/translations/qt_help_%1.qm").arg(newLocale));
        qApp->installTranslator(&mQtHelpTranslator);

        qApp->removeTranslator(&mQtXmlPatternsTranslator);
        mQtXmlPatternsTranslator.load(QString(":/translations/qtxmlpatterns_%1.qm").arg(newLocale));
        qApp->installTranslator(&mQtXmlPatternsTranslator);

        qApp->removeTranslator(&mAppTranslator);
        mAppTranslator.load(":/app_"+newLocale);
        qApp->installTranslator(&mAppTranslator);

        // Retranslate OpenCOR

        mGui->retranslateUi(this);

        // Retranslate some widgets that are not originally part of our user
        // interface

        if (mFileNewMenu)
            I18nInterface::retranslateMenu(mFileNewMenu, tr("New"));

        if (mViewWindowsMenu)
            I18nInterface::retranslateMenu(mViewWindowsMenu, tr("Windows"));

        // Update the translator of our various loaded plugins
        // Note: we must update the translator of all our plugins before we can
        //       safely retranslate them since a plugin may require another
        //       plugin to work properly...

        foreach (Plugin *plugin, mLoadedI18nPlugins)
            qobject_cast<I18nInterface *>(plugin->instance())->updateTranslator(QString(":/%1_%2").arg(plugin->name())
                                                                                                  .arg(newLocale));

        // Retranslate our various plugins

        foreach (Plugin *plugin, mLoadedI18nPlugins)
            qobject_cast<I18nInterface *>(plugin->instance())->retranslateUi();

        // Reorder our various View|Windows menu items, just in case

        reorderViewWindowsMenu();
    }

    // Update the checked menu item
    // Note: it has to be done every single time, since selecting a menu item
    //       will automatically toggle its checked status...

    mGui->actionSystem->setChecked(pRawLocale.isEmpty());

    mGui->actionEnglish->setChecked(!pRawLocale.compare(EnglishLocale));
    mGui->actionFrench->setChecked(!pRawLocale.compare(FrenchLocale));
}

//==============================================================================

void MainWindow::reorderViewWindowsMenu()
{
    // Reorder the View|Windows menu, should it exist
    // Note: this is useful after having added a new menu item or after having
    //       changed the locale

    if (mViewWindowsMenu) {
        // Retrieve the title of the menu items and keep track of their actions

        QStringList menuItemTitles;
        QMap<QString, QAction *> menuItemActions;

        foreach (QAction *menuItemAction, mViewWindowsMenu->actions()) {
            // Remove any "&" present in the menu item title, as well as replace
            // accentuated characters by non-accentuated ones, making the
            // sorting sensible

            QString menuItemTitle = menuItemAction->text().remove('&').normalized(QString::NormalizationForm_KD);

            for (int i = menuItemTitle.length()-1; i >= 0; --i) {
                if (menuItemTitle[i].category() == QChar::Mark_NonSpacing)
                    menuItemTitle.remove(i, 1);
            }

            // Keep track of the menu item title and the action to which it is
            // associated

            menuItemTitles << menuItemTitle;
            menuItemActions.insert(menuItemTitle, menuItemAction);
        }

        // Sort the menu items

        menuItemTitles.sort();

        // Add the menu items actions in the new order
        // Note: to use addAction will effectively 'move' the menu items to the
        //       end of the menu, so since we do it in the right order, we end
        //       up with the menu items being properly ordered...

        foreach (const QString &menuItemTitle, menuItemTitles)
            mViewWindowsMenu->addAction(menuItemActions.value(menuItemTitle));
    }
}

//==============================================================================

void MainWindow::updateViewWindowsMenu(QAction *pAction)
{
    // Check whether we need to insert a separator before the docked windows
    // menu item

    if (!mViewSeparator)
        mViewSeparator = mGui->menuView->insertSeparator(mGui->actionDockedWindows);

    // Check whether the View|Windows menu already exists and create it if not

    if (!mViewWindowsMenu) {
        // The View|Windows menu doesn't already exist, so create it

        mViewWindowsMenu = new QMenu(this);

        // Add the View|Windows menu to our View menu

        mGui->menuView->insertMenu(mViewSeparator, mViewWindowsMenu);
    }

    // At this stage, the View|Windows menu exist, so add the given action to
    // it

    mViewWindowsMenu->addAction(pAction);
}

//==============================================================================

void MainWindow::showSelf()
{
    // Note: to show ourselves, one would normally use activateWindow() (and
    //       possibly raise()), but depending on the operating system it may or
    //       not bring OpenCOR to the foreground, so instead we do what follows,
    //       depending on the operating system...

#if defined(Q_OS_WIN)
    // Show ourselves the Windows way

    // Retrieve OpenCOR's window Id

    HWND mainWinId = reinterpret_cast<HWND>(winId());

    // Bring OpenCOR to the foreground

    DWORD foregroundThreadProcId = GetWindowThreadProcessId(GetForegroundWindow(), 0);
    DWORD mainThreadProcId = GetWindowThreadProcessId(mainWinId, 0);

    if (foregroundThreadProcId != mainThreadProcId) {
        // OpenCOR's thread process Id is not that of the foreground window, so
        // attach the foreground thread to OpenCOR's, set OpenCOR to the
        // foreground, and detach the foreground thread from OpenCOR's

        AttachThreadInput(foregroundThreadProcId, mainThreadProcId, true);

        SetForegroundWindow(mainWinId);
        SetFocus(mainWinId);

        AttachThreadInput(foregroundThreadProcId, mainThreadProcId, false);
    } else {
        // OpenCOR's thread process Id is that of the foreground window, so
        // just set OpenCOR to the foreground

        SetForegroundWindow(mainWinId);
    }

    // Show/restore OpenCOR, depending on its current state

    if (IsIconic(mainWinId))
        ShowWindow(mainWinId, SW_RESTORE);
    else
        ShowWindow(mainWinId, SW_SHOW);

    // Note: under Windows, to use activateWindow() will only highlight the
    //       application in the taskbar, since under Windows no application
    //       should be allowed to bring itself to the foreground when another
    //       application is already in the foreground. Fair enough, but it
    //       happens that, here, the user wants OpenCOR to be brought to the
    //       foreground, hence the above code to get the effect we are after...
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    // We are on Linux or macOS, so we can simply activate the window and raise
    // ourselves

    activateWindow();
    raise();
#else
    #error Unsupported platform
#endif
}

//==============================================================================

void MainWindow::handleArguments(const QStringList &pArguments)
{
    // Handle the arguments that were passed to OpenCOR by handling them as a
    // URL if they are an OpenCOR URL or by passing them to the Core plugin,
    // should it be loaded

    QStringList arguments = QStringList();

    foreach (const QString &argument, pArguments) {
        QUrl url = argument;

        if (!url.scheme().compare("opencor"))
            handleUrl(url);
        else if (!argument.isEmpty())
            arguments << stringFromPercentEncoding(argument);
    }

    if (!arguments.isEmpty() && mCoreInterface)
        mCoreInterface->handleArguments(arguments);

    // Make sure that our status bar is shown/hidden, depending on its action's
    // status
    // Note: indeed, for some unclear reasons, to open/handle a file / OpenCOR
    //       URL, with OpenCOR not running in the first place, results in the
    //       status bar not being shown (assuming it has to be shown). So, we
    //       make sure that the status bar is shown/hidden, as needed...

    mGui->statusBar->setVisible(mGui->actionStatusBar->isChecked());
}

//==============================================================================

void MainWindow::openFileOrHandleUrl(const QString &pFileNameOrOpencorUrl)
{
    // Handle the given file name or OpenCOR URL as if it was an argument

    handleArguments(QStringList() << pFileNameOrOpencorUrl);
}

//==============================================================================

void MainWindow::handleUrl(const QUrl &pUrl)
{
    // Handle the action that was passed to OpenCOR

    QString actionName = pUrl.authority();

    if (!actionName.compare("openPluginsDialog", Qt::CaseInsensitive)) {
        // We want to open the Plugins dialog

        actionPluginsTriggered();
    } else if (!actionName.compare("openPreferencesDialog", Qt::CaseInsensitive)) {
        // We want to open the Preferences dialog

        showPreferencesDialog(urlArguments(pUrl));
    } else if (!actionName.compare("openAboutDialog", Qt::CaseInsensitive)) {
        // We want to open the About dialog

        actionAboutTriggered();
    } else if (!actionName.compare("openFile", Qt::CaseInsensitive)) {
        // We want to open a file, so handle it as an argument that is passed to
        // OpenCOR
        // Note: the file name is contained in the path of the URL minus the
        //       leading forward slash. Indeed, an open file request looks like
        //       opencor://openFile//home/user/file...

        handleArguments(QStringList() << urlArguments(pUrl));
    } else if (!actionName.compare("openFiles", Qt::CaseInsensitive)) {
        // We want to open some files, so handle them as a series of arguments
        // that were passed to OpenCOR
        // Note: the file names are contained in the path of the URL minus the
        //       leading forward slash. Indeed, an open files request looks like
        //       opencor://openFiles//home/user/file1|/home/user/file2...

        handleArguments(urlArguments(pUrl).split('|'));
    } else {
        // We are dealing with an action that OpenCOR itself can't handle, but
        // maybe one of its loaded plugins can

        QString pluginName = actionName.split('.').first();

        foreach (Plugin *plugin, mLoadedPluginPlugins) {
            if (!plugin->name().compare(pluginName, Qt::CaseInsensitive)) {
                // This is an action for the current plugin, so forward the
                // action to it, should it support the Plugin interface

                PluginInterface *pluginInterface = qobject_cast<PluginInterface *>(plugin->instance());

                if (pluginInterface)
                    pluginInterface->handleUrl(pUrl);

                break;
            }
        }
    }
}

//==============================================================================

void MainWindow::handleMessage(const QString &pMessage)
{
    // We have just received a message, which means that the user tried to run
    // another instance of OpenCOR, which sent a message to this instance,
    // asking it to bring itself to the foreground and handling all the
    // arguments passed in the given message

    showSelf();

    handleArguments(pMessage.split('|'));
}

//==============================================================================

void MainWindow::actionFullScreenTriggered()
{
    // Switch to / back from full screen mode

    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

//==============================================================================

void MainWindow::actionSystemTriggered()
{
    // Select the system's language as the language used by OpenCOR

    setLocale();
}

//==============================================================================

void MainWindow::actionEnglishTriggered()
{
    // Select English as the language used by OpenCOR

    setLocale(EnglishLocale);
}

//==============================================================================

void MainWindow::actionFrenchTriggered()
{
    // Select French as the language used by OpenCOR

    setLocale(FrenchLocale);
}

//==============================================================================

void MainWindow::actionPluginsTriggered()
{
    if (mPluginManager->plugins().count()) {
        // There are some plugins, so we can show the plugins dialog

        mSettings->beginGroup("PluginsDialog");
            PluginsDialog pluginsDialog(mSettings, mPluginManager, this);

            pluginsDialog.exec();
        mSettings->endGroup();

        // Restart OpenCOR (after having saved its settings) in case the user
        // asked for his/her plugin-related settings to be  applied

        if (pluginsDialog.result() == QMessageBox::Apply)
            restart(true);
    } else {
        warningMessageBox(tr("Plugins"),
                          tr("No plugins could be found."));
    }
}

//==============================================================================

void MainWindow::showPreferencesDialog(const QString &pPluginName)
{
    // Show the preferences dialog, if we have at least one plugin that supports
    // the Preferences interface

    if (mPluginManager->plugins().count()) {
        if (mLoadedPreferencesPlugins.count()) {
            mSettings->beginGroup("PreferencesDialog");
                PreferencesDialog preferencesDialog(mSettings, mPluginManager,
                                                    pPluginName, this);

                preferencesDialog.exec();
            mSettings->endGroup();

            // Let people know about the plugins that had their preferences
            // changed, if any and if requested

            if (    (preferencesDialog.result() == QMessageBox::Ok)
                && !preferencesDialog.pluginNames().isEmpty()) {
                foreach (Plugin *plugin, mPluginManager->loadedPlugins()) {
                    PreferencesInterface *preferencesInterface = qobject_cast<PreferencesInterface *>(plugin->instance());

                    if (preferencesInterface)
                        preferencesInterface->preferencesChanged(preferencesDialog.pluginNames());
                }
            }
        } else {
            warningMessageBox(tr("Preferences"),
                              tr("No plugins have preferences."));
        }
    } else {
        warningMessageBox(tr("Preferences"),
                          tr("No plugins could be found."));
    }
}

//==============================================================================

void MainWindow::actionPreferencesTriggered()
{
    // Show the preferences dialog

    showPreferencesDialog();
}

//==============================================================================

void MainWindow::actionHomePageTriggered()
{
    // Look up OpenCOR's home page

    QDesktopServices::openUrl(HomePageUrl);
}

//==============================================================================

void MainWindow::actionCheckForUpdatesTriggered()
{
    // Show the check for updates dialog

    mSettings->beginGroup(SettingsCheckForUpdatesDialog);
        CheckForUpdatesDialog checkForUpdatesDialog(mSettings, mApplicationDate, this);

        checkForUpdatesDialog.exec();
    mSettings->endGroup();
}

//==============================================================================

void MainWindow::actionAboutTriggered()
{
    // Display some information about OpenCOR

    aboutMessageBox(tr("About"),
                    "<h1 align=center><strong>"+version()+"</strong></h1>"
                    "<h3 align=center><em>"+QSysInfo::prettyProductName()+"</em></h3>"
                    "<p align=center><em>"+copyright()+"</em></p>"
                    "<p>"+applicationDescription()+"</p>");
}

//==============================================================================

void MainWindow::restart(bool pSaveSettings) const
{
    // Restart OpenCOR after saving its settings, if required
    // Note: the closeEvent method won't get called when exiting OpenCOR and
    //       this is exactly what we want in case we don't want to save its
    //       settings (e.g. when resetting all)

    if (pSaveSettings)
        saveSettings();

    qApp->exit(pSaveSettings?NormalRestart:CleanRestart);
}

//==============================================================================

void MainWindow::showEnableActions(const QList<QAction *> &pActions)
{
    // Show/enable or hide/disable the given actions, depending on whether they
    // correspond to a menu with visible/enabled or hidden/disabled actions,
    // respectively

    foreach (QAction *action, pActions) {
        QMenu *actionMenu = action->menu();

        if (actionMenu) {
            QList<QAction *> actionMenuActions = actionMenu->actions();

            showEnableActions(actionMenuActions);

            bool showEnable = false;

            foreach (QAction *actionMenuAction, actionMenuActions) {
                if (   !actionMenuAction->isSeparator()
                    &&  actionMenuAction->isVisible()) {
                    showEnable = true;

                    break;
                }
            }

            showEnableAction(action, showEnable);
        }
    }
}

//==============================================================================

void MainWindow::updateGui(OpenCOR::Plugin *pViewPlugin,
                           const QString &pFileName)
{
    // We come here as a result of our central widget having updated its GUI,
    // meaning that a new view or file has been selected, so we may need to
    // enable/disable and/or show/hide some menus/actions/etc.

    // Keep track of our view plugin

    mViewPlugin = pViewPlugin;

    // Let our different plugins know that the GUI has been updated
    // Note: this can be useful when a plugin (e.g. CellMLTools) offers some
    //       tools that may need to be enabled/disabled and shown/hidden,
    //       depending on which view plugin and/or file are currently active...

    foreach (Plugin *plugin, mLoadedGuiPlugins)
        qobject_cast<GuiInterface *>(plugin->instance())->updateGui(mViewPlugin, pFileName);

    // Go through our different menus and show/hide them, depending on whether
    // they have visible items

    showEnableActions(mGui->menuBar->actions());
}

//==============================================================================

void MainWindow::showDockedWindows(bool pShow, bool pInitialisation)
{
    // Show/hide the docked windows

    if (!pInitialisation) {
        if (!pShow)
            mDockedWindowsState = saveState();

        foreach (Plugin *plugin, mLoadedWindowPlugins) {
            WindowInterface *windowInterface = qobject_cast<WindowInterface *>(plugin->instance());

            if (!windowInterface->windowWidget()->isFloating())
                windowInterface->windowWidget()->setVisible(pShow);
        }

        if (pShow && !mDockedWindowsState.isEmpty())
            restoreState(mDockedWindowsState);
    }

    // Keep track of the docked windows visible state

    mDockedWindowsVisible = pShow;

    // Update the checked state of our docked windows action

    mGui->actionDockedWindows->setChecked(pShow);
}

//==============================================================================

void MainWindow::showDockedWindows(bool pShow)
{
    // Show/hide the docked windows

    showDockedWindows(pShow, false);
}

//==============================================================================

void MainWindow::toggleDockedWindows()
{
    // Toggle the visible state of our docked windows

    showDockedWindows(!mGui->actionDockedWindows->isChecked());
}

//==============================================================================

void MainWindow::updateDockWidgetsVisibility()
{
    // Check whether at least one dockable window is visible

    mDockedWindowsVisible = false;

    foreach (Plugin *plugin, mLoadedWindowPlugins) {
        QDockWidget *dockWidget = qobject_cast<WindowInterface *>(plugin->instance())->windowWidget();

        if (!dockWidget->isFloating() && dockWidget->isVisible()) {
            mDockedWindowsVisible = true;

            break;
        }
    }

    // Update the checked state of our docked windows action

    mGui->actionDockedWindows->setChecked(mDockedWindowsVisible);
}

//==============================================================================

void MainWindow::resetAll()
{
    if (questionMessageBox(qAppName(),
                           tr("You are about to reset <strong>all</strong> of your settings. Do you want to proceed?")) == QMessageBox::Yes ) {
        // Restart OpenCOR without first saving its settings

        restart(false);
    }
}

//==============================================================================

}   // namespace OpenCOR

//==============================================================================
// End of file
//==============================================================================
