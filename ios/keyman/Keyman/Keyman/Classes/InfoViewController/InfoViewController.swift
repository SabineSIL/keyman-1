//
//  InfoViewController.swift
//  Keyman
//
//  Created by Gabriel Wong on 2017-09-05.
//  Copyright © 2017 SIL International. All rights reserved.
//

import KeymanEngine
import UIKit
import Reachability

class InfoViewController: UIViewController, UIWebViewDelegate {
  @IBOutlet var webView: UIWebView!

  private var networkReachable: Reachability?

  convenience init() {
    if UIDevice.current.userInterfaceIdiom == .phone {
      self.init(nibName: "InfoViewController_iPhone", bundle: nil)
    } else {
      self.init(nibName: "InfoViewController_iPad", bundle: nil)
    }
  }

  override func viewDidLoad() {
    super.viewDidLoad()
    extendedLayoutIncludesOpaqueBars = true
    webView?.delegate = self
    NotificationCenter.default.addObserver(self, selector: #selector(self.networkStatusChanged),
        name: NSNotification.Name.reachabilityChanged, object: nil)

    do {
      try networkReachable = Reachability(hostname: "keyman.com")
      try networkReachable?.startNotifier()
    } catch {
      SentryManager.captureAndLog(error, message: "error starting Reachability notifier: \(error)")
    }
  }

  @objc func networkStatusChanged(_ notification: Notification) {
    reloadKeymanHelp()
  }

  func reloadKeymanHelp() {
    loadFromLocal()
  }

  private func loadFromLocal() {
    let offlineHelpBundle = Bundle(path: Bundle.main.path(forResource: "OfflineHelp", ofType: "bundle")!)!

    // Safari won't recognize the contents without the .html ending.
    let filePath = offlineHelpBundle.path(forResource: "index", ofType: "html", inDirectory: nil)
    webView.loadRequest(URLRequest(url: URL.init(fileURLWithPath: filePath!)))
  }

  // Currently unused, as we haven't yet added a toggle to allow users to choose online
  // over the default of offline.
  private func loadFromServer() {
    let appVersion = Version.current.majorMinor
    let url =  "\(KeymanHosts.HELP_KEYMAN_COM)/products/iphone-and-ipad/\(appVersion.plainString)/"
    webView.loadRequest(URLRequest(url: URL(string: url)!))
    log.debug("Info page URL: \(url)")
  }
}
