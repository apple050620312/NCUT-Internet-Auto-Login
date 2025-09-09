#include "Network.h"

#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QRegularExpression>
#include <QUrlQuery>

namespace Net {

bool checkConnection(int timeoutMs) {
  QTcpSocket sock;
  sock.connectToHost("1.1.1.1", 53);
  return sock.waitForConnected(timeoutMs);
}

static QByteArray httpGet(const QUrl& url) {
  QNetworkAccessManager mgr;
  QNetworkRequest req(url);
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
  QNetworkReply* rep = mgr.get(req);
  QEventLoop loop; QObject::connect(rep, &QNetworkReply::finished, &loop, &QEventLoop::quit); loop.exec();
  QByteArray body = rep->readAll(); rep->deleteLater(); return body;
}

static QByteArray httpPost(const QUrl& url, const QByteArray& body, const QUrl& referer, const QUrl& origin) {
  QNetworkAccessManager mgr;
  QNetworkRequest req(url);
  req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
  req.setRawHeader("Upgrade-Insecure-Requests", "1");
  if (referer.isValid()) req.setRawHeader("Referer", referer.toString().toUtf8());
  if (origin.isValid()) req.setRawHeader("Origin", origin.toString().toUtf8());
  QNetworkReply* rep = mgr.post(req, body);
  QEventLoop loop; QObject::connect(rep, &QNetworkReply::finished, &loop, &QEventLoop::quit); loop.exec();
  QByteArray resp = rep->readAll(); rep->deleteLater(); return resp;
}

static QString extractRedirectUrl(const QByteArray& page) {
  QString html = QString::fromUtf8(page);
  QRegularExpression re("window\\.location=\\\"(http://\\d+\\.\\d+\\.\\d+\\.\\d+:1000/fgtauth\\?[^\\\"]+)\\\"");
  auto m = re.match(html);
  if (m.hasMatch()) return m.captured(1);
  return {};
}

static QString extractGatewayIp(const QString& redir) {
  QRegularExpression re("http://(\\d+\\.\\d+\\.\\d+\\.\\d+):1000");
  auto m = re.match(redir);
  if (m.hasMatch()) return m.captured(1);
  return {};
}

static QString extractMagic(const QString& redir) {
  QRegularExpression re("fgtauth\\?([^&]+)");
  auto m = re.match(redir);
  if (m.hasMatch()) return m.captured(1);
  return {};
}

LoginResult login(const QString& account, const QString& password) {
  LoginResult r;
  QByteArray page = httpGet(QUrl("http://www.gstatic.com/generate_204"));
  if (page.isEmpty()) { r.message = "初始請求失敗"; return r; }
  const QString redirectUrl = extractRedirectUrl(page);
  if (redirectUrl.isEmpty()) { r.message = "無重導URL"; return r; }
  const QString gw = extractGatewayIp(redirectUrl);
  if (gw.isEmpty()) { r.message = "無閘道IP"; return r; }
  const QString magic = extractMagic(redirectUrl);
  if (magic.isEmpty()) { r.message = "無 magic"; return r; }

  QUrl origin(QString("http://%1:1000").arg(gw));
  QUrl referer(redirectUrl);
  QByteArray body;
  body += "4Tredir=" + QUrl::toPercentEncoding("http://www.gstatic.com/generate_204");
  body += "&magic=" + QUrl::toPercentEncoding(magic);
  body += "&username=" + QUrl::toPercentEncoding(account);
  body += "&password=" + QUrl::toPercentEncoding(password);
  QByteArray resp = httpPost(QUrl(QString("http://%1:1000/").arg(gw)), body, referer, origin);
  const QString low = QString::fromUtf8(resp).toLower();
  if (low.contains("/keepalive?")) { r.success = true; r.message = "登入成功"; }
  else { r.message = "登入失敗"; }
  return r;
}

}

