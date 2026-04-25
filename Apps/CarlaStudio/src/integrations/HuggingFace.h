// Copyright (C) 2026 Abdul, Hashim.
//
// This file is part of CARLA Studio.
// Licensed under the GNU Affero General Public License v3 or later —
// see <LICENSE> at the project root or
// <https://www.gnu.org/licenses/agpl-3.0.html> for the full text.
// SPDX-License-Identifier: AGPL-3.0-or-later

#pragma once

#include <QObject>
#include <QProcessEnvironment>
#include <QString>

#include <functional>

class QProcess;

namespace carla_studio::integrations::hf {

struct AccountInfo {
  bool    ok = false;
  int     httpStatus = 0;
  QString user;
  QString fullname;
  QString email;
  QString plan;
  QStringList orgs;
  QString rawError;
};

QString  loadToken();
void     saveToken(const QString &token);
void     clearToken();
bool     hasToken();

QProcessEnvironment &injectIntoEnv(QProcessEnvironment &env);

QProcess *verifyAsync(QObject *parent,
                      const QString &token,
                      std::function<void(AccountInfo)> done);

}
