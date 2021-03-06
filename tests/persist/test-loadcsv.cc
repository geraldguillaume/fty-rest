/*
 *
 * Copyright (C) 2015 - 2018 Eaton
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

/*!
 * \file test-loadcsv.cc
 * \author Michal Vyskocil <MichalVyskocil@Eaton.com>
 * \author Tomas Halman <TomasHalman@Eaton.com>
 * \author Alena Chernikava <AlenaChernikava@Eaton.com>
 * \brief Not yet documented file
 */
#include <catch.hpp>
#include <fstream>
#include <cxxtools/csvdeserializer.h>
#include <tntdb/result.h>
#include <tntdb/statement.h>
#include <fty_common.h>

#include "csv.h"
#include "db/inout.h"
#include "assetcrud.h"

using namespace persist;

std::string get_dc_lab_description() {
    tntdb::Connection conn;
    REQUIRE_NOTHROW ( conn = tntdb::connectCached(url) );

    tntdb::Statement st = conn.prepareCached(
        " SELECT value from t_bios_asset_ext_attributes"
        " WHERE id_asset_element in ("
        "   SELECT id_asset_element FROM t_bios_asset_element"
        "   WHERE NAME = 'DC-LAB')"
        " AND keytag='description'"
    );

    // this is ugly, but was lazy to read the docs
    tntdb::Result result = st.select();
    for ( auto &row: result )
    {
        std::string ret;
        row[0].get(ret);
        return ret;
    }

    return std::string{"could-not-get-here"};  //to make gcc happpy
}

inline std::string to_utf8(const cxxtools::String& ws) {
    return cxxtools::Utf8Codec::encode(ws);
}

TEST_CASE("CSV multiple field names", "[csv]") {

    std::string base_path{__FILE__};
    std::string csv = base_path + ".csv";
    std::string tsv = base_path + ".tsv";
    std::string ssv = base_path + ".ssv";
    std::string usv = base_path + ".usv";
    std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> okRows;
    std::map <int, std::string> failRows;

    static std::string exp = to_utf8(cxxtools::String(L"Lab DC(тест)"));
    REQUIRE_NOTHROW(get_dc_lab_description() == exp);

    auto void_fn = []() {return;};

    // test coma separated values format
    std::fstream csv_buf{csv};
    REQUIRE_NOTHROW(load_asset_csv(csv_buf, okRows, failRows, void_fn ));

    // test tab separated values
    std::fstream tsv_buf{tsv};
    REQUIRE_NOTHROW(load_asset_csv(tsv_buf, okRows, failRows, void_fn ));

    // test semicolon separated values
    std::fstream ssv_buf{ssv};
    REQUIRE_NOTHROW(load_asset_csv(ssv_buf, okRows, failRows, void_fn ));

    // test underscore separated values
    std::fstream usv_buf{usv};
    REQUIRE_THROWS_AS (load_asset_csv(usv_buf, okRows, failRows, void_fn ), std::invalid_argument);
}

TEST_CASE("CSV bug 661 - segfault with quote in name", "[csv]") {

    std::string base_path{__FILE__};
    std::string csv = base_path + ".661.csv";
    std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> okRows;
    std::map <int, std::string> failRows;

    auto void_fn = []() {return;};

    std::fstream csv_buf{csv};
    REQUIRE_THROWS_AS(load_asset_csv(csv_buf, okRows, failRows, void_fn), std::out_of_range);
}

TEST_CASE("CSV bug 661 - segfault on csv without mandatory columns", "[csv]") {

    std::string base_path{__FILE__};
    std::string csv = base_path + ".661.2.csv";
    std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> okRows;
    std::map <int, std::string> failRows;

    auto void_fn = []() {return;};
    std::fstream csv_buf{csv};
    REQUIRE_THROWS_AS(load_asset_csv(csv_buf, okRows, failRows, void_fn), std::invalid_argument);
}

TEST_CASE("CSV bug 661 - segfault ...", "[csv]") {

    std::string base_path{__FILE__};
    std::string csv = base_path + ".group3.csv";
    std::vector <std::pair<db_a_elmnt_t,persist::asset_operation>> okRows;
    std::map <int, std::string> failRows;

    auto void_fn = []() {return;};
    std::fstream csv_buf{csv};
    REQUIRE_NOTHROW(load_asset_csv(csv_buf, okRows, failRows, void_fn));
}
