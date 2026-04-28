// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "vehicle_import/ImporterClient.h"

#include <QByteArray>
#include <QJsonDocument>

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace carla_studio::vehicle_import {

bool probeImporterPort() {
  int sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return false;
  struct sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(kImporterPort);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  const int flags = fcntl(sock, F_GETFL, 0);
  fcntl(sock, F_SETFL, flags | O_NONBLOCK);

  bool ok = false;
  const int r = ::connect(sock, (struct sockaddr *)&addr, sizeof(addr));
  if (r == 0) {
    ok = true;
  } else if (errno == EINPROGRESS) {
    fd_set wfds; FD_ZERO(&wfds); FD_SET(sock, &wfds);
    struct timeval tv{0, 200 * 1000};
    if (::select(sock + 1, nullptr, &wfds, nullptr, &tv) > 0) {
      int err = 0;
      socklen_t len = sizeof(err);
      getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
      ok = (err == 0);
    }
  }
  ::close(sock);
  return ok;
}

QString sendJson(const QJsonObject &spec) {
  const QByteArray payload = QJsonDocument(spec).toJson(QJsonDocument::Compact);

  int sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) return QString();

  struct sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(kImporterPort);
  inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

  if (::connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
    ::close(sock);
    return QString();
  }

  const quint32 len = (quint32)payload.size();
  unsigned char hdr[4] = {
    (unsigned char)(len & 0xFF),
    (unsigned char)((len >> 8)  & 0xFF),
    (unsigned char)((len >> 16) & 0xFF),
    (unsigned char)((len >> 24) & 0xFF),
  };
  if (::send(sock, hdr, 4, 0) != 4) { ::close(sock); return QString(); }
  if (::send(sock, payload.constData(), payload.size(), 0) != payload.size()) {
    ::close(sock);
    return QString();
  }

  unsigned char rhdr[4] = {0};
  ssize_t n = 0;
  while (n < 4) {
    const ssize_t r = ::recv(sock, rhdr + n, 4 - n, 0);
    if (r <= 0) { ::close(sock); return QString(); }
    n += r;
  }
  const quint32 rlen = (quint32)rhdr[0]
                     | ((quint32)rhdr[1] <<  8)
                     | ((quint32)rhdr[2] << 16)
                     | ((quint32)rhdr[3] << 24);
  if (rlen == 0 || rlen > 10u * 1024u * 1024u) { ::close(sock); return QString(); }

  QByteArray body(rlen, 0);
  ssize_t got = 0;
  while (got < (ssize_t)rlen) {
    const ssize_t r = ::recv(sock, body.data() + got, rlen - got, 0);
    if (r <= 0) { ::close(sock); return QString(); }
    got += r;
  }
  ::close(sock);
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
