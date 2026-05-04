// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#include "HuggingFace.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSettings>

namespace carla_studio::integrations::hf {

namespace {

constexpr const char *kTokenKey = "hf/token";

constexpr quint8 kXorNonce[] = {0x37, 0x6d, 0xa1, 0xb2, 0x4f, 0xe9, 0x18, 0xc5};

QByteArray scramble(const QByteArray &in) {
  QByteArray out;
  out.reserve(in.size());
  for (int i = 0; i < in.size(); ++i) {
    out.append(static_cast<char>(static_cast<quint8>(in[i]) ^
                                  kXorNonce[i % sizeof(kXorNonce)]));
  }
  return out;
}

}

QString loadToken() {
  QSettings s;
  const QByteArray scrambled = QByteArray::fromBase64(
      s.value(kTokenKey, QByteArray()).toByteArray());
  if (scrambled.isEmpty()) return QString();
  return QString::fromUtf8(scramble(scrambled));
}

void saveToken(const QString &token) {
  QSettings s;
  if (token.isEmpty()) {
    s.remove(kTokenKey);
    return;
  }
  s.setValue(kTokenKey,
             scramble(token.toUtf8()).toBase64());
}

void clearToken() {
  QSettings s;
  s.remove(kTokenKey);
}

bool hasToken() {
  QSettings s;
  return !s.value(kTokenKey).toByteArray().isEmpty();
}

QProcessEnvironment &injectIntoEnv(QProcessEnvironment &env) {
  const QString tok = loadToken();
  if (!tok.isEmpty()) env.insert("HF_TOKEN", tok);
  return env;
}

QProcess *verifyAsync(QObject *parent,
                      const QString &token,
                      std::function<void(AccountInfo)> done) {
  QProcess *proc = new QProcess(parent);
  proc->setProcessChannelMode(QProcess::SeparateChannels);

  QStringList args = {
    "-sS",
    "--max-time", "12",
    "-w", "\n%{http_code}",
    "-H", "Accept: application/json",
    "--config", "-",
    "https://huggingface.co/api/whoami-v2",
  };
  QString sanitized = token;
  sanitized.replace('\\', QStringLiteral("\\\\"));
  sanitized.replace('"',  QStringLiteral("\\\""));
  const QByteArray curlConfig =
      QStringLiteral("header = \"Authorization: Bearer %1\"\n")
        .arg(sanitized).toUtf8();

  QObject::connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
    parent, [proc, done](int , QProcess::ExitStatus exitStatus) {
      AccountInfo info;
      const QByteArray out = proc->readAllStandardOutput();
      const QByteArray err = proc->readAllStandardError();

      if (exitStatus != QProcess::NormalExit) {
        info.rawError = QStringLiteral("curl exited abnormally");
        if (done) done(info);
        proc->deleteLater();
        return;
      }

      const int lastNewline = out.lastIndexOf('\n');
      QByteArray bodyBytes = lastNewline >= 0 ? out.left(lastNewline) : QByteArray();
      const QString statusStr = lastNewline >= 0
          ? QString::fromLatin1(out.mid(lastNewline + 1)).trimmed()
          : QString();
      info.httpStatus = statusStr.toInt();

      if (info.httpStatus != 200) {
        info.rawError = bodyBytes.isEmpty()
            ? QString::fromLocal8Bit(err).trimmed()
            : QString::fromUtf8(bodyBytes).trimmed();
        if (done) done(info);
        proc->deleteLater();
        return;
      }

      QJsonParseError perr;
      const QJsonDocument doc = QJsonDocument::fromJson(bodyBytes, &perr);
      if (perr.error != QJsonParseError::NoError || !doc.isObject()) {
        info.rawError = QStringLiteral("Could not parse whoami response: %1")
                          .arg(perr.errorString());
        if (done) done(info);
        proc->deleteLater();
        return;
      }
      const QJsonObject obj = doc.object();
      info.ok       = true;
      info.user     = obj.value("name").toString();
      info.fullname = obj.value("fullname").toString();
      info.email    = obj.value("email").toString();
      info.plan     = obj.value("plan").toString();
      for (const QJsonValue &v : obj.value("orgs").toArray()) {
        if (v.isObject()) {
          const QString n = v.toObject().value("name").toString();
          if (!n.isEmpty()) info.orgs.append(n);
        }
      }
      if (done) done(info);
      proc->deleteLater();
    });

  QObject::connect(proc, &QProcess::errorOccurred,
    parent, [proc, done](QProcess::ProcessError) {
      AccountInfo info;
      info.rawError = QStringLiteral("Could not invoke curl. Is it installed?");
      if (done) done(info);
      proc->deleteLater();
    });

  proc->start("curl", args);
  if (proc->waitForStarted(5000)) {
    proc->write(curlConfig);
    proc->closeWriteChannel();
  }
  return proc;
}

}
