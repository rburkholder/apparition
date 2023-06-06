/************************************************************************
 * Copyright(c) 2023, One Unified. All rights reserved.                 *
 * email: info@oneunified.net                                           *
 *                                                                      *
 * This file is provided as is WITHOUT ANY WARRANTY                     *
 *  without even the implied warranty of                                *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                *
 *                                                                      *
 * This software may not be used nor distributed without proper license *
 * agreement.                                                           *
 *                                                                      *
 * See the file LICENSE.txt for redistribution information.             *
 ************************************************************************/

/*
 * File:    FileNotify.hpp
 * Author:  raymond@burkholder.net
 * Project: Apparition
 * Created: 2023/06/05 18:46:39
 */

#include <thread>
#include <functional>
#include <string_view>

class FileNotify {
public:

  enum class EType { unknown_, delete_, create_, modify_ };
  using fNotify_t = std::function<void( EType, const std::string_view& )>;

  FileNotify(
    fNotify_t&& fConfig
  , fNotify_t&& fScript
  );
  ~FileNotify();

protected:
private:

  int m_fdINotify;
  int m_wdScript;
  int m_wdConfig;

  bool m_bActive;
  std::thread m_threadINotify;

  fNotify_t m_fNotifyConfig;
  fNotify_t m_fNotifyScript;

  void Close();
};
