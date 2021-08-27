//  This file is distributed under the BSD 3-Clause License. See LICENSE for details.

#include "likely.hpp"
#include "lnast.hpp"
#include "symbol_table.hpp"

bool Symbol_table::var(mmap_lib::str key) {
  auto [var, field] = get_var_field(key);

  const auto it = varmap.find(std::pair(stack.back().scope, var));
  if (unlikely(it != varmap.end())) {
    Lnast::info("re-declaring {} which already exists in {}", var, stack.back().scope);
    return false;
  }

  auto bundle = std::make_shared<Bundle>(var);
  bundle->var(field, Lconst::invalid());
  varmap.insert({std::pair(stack.back().scope,var), bundle});
  return true;
}

bool Symbol_table::mut(mmap_lib::str key, const Lconst &trivial) {
  auto [var, field] = get_var_field(key);

  const auto it = varmap.find(std::pair(stack.back().scope, var));
  if (unlikely(it == varmap.end())) {
    Lnast::info("mutate {} but variable not declared in {}", var, stack.back().scope);
    return false;
  }

  if (unlikely(!it->second->has_trivial(field))) {
    Lnast::info("mutate {} but field {} not declared in {}", var, field, stack.back().scope);
    return false;
  }

  it->second->mut(field, trivial);

  return true;
}

bool Symbol_table::let(mmap_lib::str key, std::shared_ptr<Bundle> bundle) {
  auto [var, field] = get_var_field(key);

  const auto it = varmap.find(std::pair(stack.back().scope, var));
  if (unlikely(it != varmap.end())) {
    Lnast::info("let {} but variable already declared in {}", var, stack.back().scope);
    return false;
  }

  bundle->set_immutable();
  varmap.insert({std::pair(stack.back().scope,var), bundle});
  return true;
}

void Symbol_table::always_scope() {
  I(!stack.empty());

  // WARNING: keep same scope because shadowing can not happen
  stack.emplace_back(Scope(Scope_type::Always, stack.back().func_id, stack.back().scope));
}

void Symbol_table::funcion_scope(mmap_lib::str func_id, std::shared_ptr<Bundle> inp_bundle) {

  mmap_lib::str scope = func_id;
  for(int i=stack.size()-1;i>=0;--i) {
    const auto &s=stack[i];
    if (s.func_id != func_id)
      continue;
    I(s.scope.back() != '/');
    scope   = mmap_lib::str::concat(s.scope, "/", func_id);
    break;
  }

  stack.emplace_back(Scope(Scope_type::Function, func_id, scope));

  if (inp_bundle) {
    auto ok = let("$", inp_bundle);
    I(ok);
  }
}

std::shared_ptr<Bundle> Symbol_table::leave_scope() {
  I(!stack.empty());

  std::shared_ptr<Bundle> outputs;
  if (stack.back().type == Scope_type::Function) {
    auto it = varmap.find(std::pair(stack.back().scope, "%"_str));
    if (it != varmap.end()) {
      I(has_bundle("%"));
      outputs = it->second;
    }
  }

  if (stack.size()==1) { // Just clear everything and be done
    I(stack.back().type == Scope_type::Function);

    varmap.clear();
    stack.clear();
    return outputs;
  }

  auto scope = stack.back().scope;

  for(auto var:stack.back().declared) {
    varmap.erase(std::pair(scope, var));
  }

  stack.pop_back();

  return outputs;
}

bool Symbol_table::has_trivial(mmap_lib::str key) const {
  auto [var, field] = get_var_field(key);

  const auto it = varmap.find(std::pair(stack.back().scope, var));
  if (it == varmap.end())
    return false;

  return it->second->has_trivial(field);
}

Lconst Symbol_table::get_trivial(mmap_lib::str key) const {
  auto [var, field] = get_var_field(key);

  const auto it = varmap.find(std::pair(stack.back().scope, var));
  if (it == varmap.end())
    return false;

  return it->second->get_trivial(field);
}

bool Symbol_table::has_bundle(mmap_lib::str key) const {
  auto [var, field] = get_var_field(key);

  const auto it = varmap.find(std::pair(stack.back().scope, var));
  if (it == varmap.end())
    return false;

  return field.empty() || it->second->has_bundle(field);
}

