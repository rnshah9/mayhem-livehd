//  This file is distributed under the BSD 3-Clause License. See LICENSE for details.
#pragma once

#include "lconst.hpp"
#include "lgtuple.hpp"
#include "node.hpp"
#include "pass.hpp"

class Gioc {
private:
  std::string_view path;
  absl::flat_hash_map<std::string, Node_pin> name2dpin;      
  absl::flat_hash_map<std::string, Node_pin> field2dpin;
protected:

  std::vector<std::string_view> split_name (std::string_view hier_name, std::string_view delimiter);
  bool     subgraph_outp_is_tuple (Sub_node* sub);
  void     subgraph_io_connection (LGraph *lg, Sub_node* sub, std::string_view arg_tup_name, std::string_view res_name, Node subg_node);
  Node_pin setup_tuple_ref        (LGraph *lg, std::string_view tup_name);
  Node_pin setup_field_dpin       (LGraph *lg, std::string_view key_name);

public:
  Gioc (std::string_view _path);
  void do_trans(LGraph *lg);
};
