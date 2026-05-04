// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "vehicle_import/ImporterClient.h"

#include <QByteArray>
#include <QHostAddress>
#include <QJsonDocument>
#include <QTcpSocket>

namespace carla_studio::vehicle_import {

namespace {
constexpr int kProbeTimeoutMs = 200;
constexpr int kIoTimeoutMs    = 30000;
constexpr quint32 kMaxBody    = 10u * 1024u * 1024u;
}

bool probeImporterPort() {
  QTcpSocket sock;
  sock.connectToHost(QHostAddress::LocalHost, kImporterPort);
  return sock.waitForConnected(kProbeTimeoutMs);
}

QString sendJson(const QJsonObject &spec) {
  const QByteArray payload = QJsonDocument(spec).toJson(QJsonDocument::Compact);

  QTcpSocket sock;
  sock.connectToHost(QHostAddress::LocalHost, kImporterPort);
  if (!sock.waitForConnected(kIoTimeoutMs)) return QString();

  const quint32 len = static_cast<quint32>(payload.size());
  unsigned char hdr[4] = {
    static_cast<unsigned char>(len & 0xFF),
    static_cast<unsigned char>((len >> 8)  & 0xFF),
    static_cast<unsigned char>((len >> 16) & 0xFF),
    static_cast<unsigned char>((len >> 24) & 0xFF),
  };

  auto writeAll = [&](const char *data, qint64 total) -> bool {
    qint64 sent = 0;
    while (sent < total) {
      const qint64 n = sock.write(data + sent, total - sent);
      if (n < 0) return false;
      sent += n;
      if (!sock.waitForBytesWritten(kIoTimeoutMs)) return false;
    }
    return true;
  };

  if (!writeAll(reinterpret_cast<const char *>(hdr), 4)) return QString();
  if (!writeAll(payload.constData(), payload.size()))    return QString();

  unsigned char rhdr[4] = {0};
  qint64 n = 0;
  while (n < 4) {
    if (sock.bytesAvailable() <= 0 && !sock.waitForReadyRead(kIoTimeoutMs))
      return QString();
    const qint64 r = sock.read(reinterpret_cast<char *>(rhdr) + n, 4 - n);
    if (r <= 0) return QString();
    n += r;
  }
  const quint32 rlen = static_cast<quint32>(rhdr[0])
                     | (static_cast<quint32>(rhdr[1]) <<  8)
                     | (static_cast<quint32>(rhdr[2]) << 16)
                     | (static_cast<quint32>(rhdr[3]) << 24);
  if (rlen == 0 || rlen > kMaxBody) return QString();

  QByteArray body(rlen, 0);
  qint64 got = 0;
  while (got < static_cast<qint64>(rlen)) {
    if (sock.bytesAvailable() <= 0 && !sock.waitForReadyRead(kIoTimeoutMs))
      return QString();
    const qint64 r = sock.read(body.data() + got, rlen - got);
    if (r <= 0) return QString();
    got += r;
  }
  return QString::fromUtf8(body);
}

QJsonObject buildSpawnSpec(const QString &assetPath,
                           double x, double y, double z, double yaw) {
  QJsonObject o;
  o["action"]     = QStringLiteral("spawn");
  o["asset_path"] = assetPath;
  o["x"] = x; o["y"] = y; o["z"] = z;
  o["yaw"] = yaw;
  return o;
}

}  // namespace carla_studio::vehicle_import
