#include <utility>

#include "StringUtility.h"
#include "CommandNode.h"

namespace comad::command {
	using namespace logger;
	using namespace build_options;

	CommandNode::CommandNode() = default;

	CommandNode::CommandNode(std::reference_wrapper<CommandNode> parent, std::string_view name) :
		parent_{ &parent.get() },
		name_{ name }
	{}

	CommandNode::CommandNode(CommandTemplate cmd_template, CommandExecutor executor) :
		cmd_template_{ std::move(cmd_template) },
		executor_{ executor }
	{}

	void CommandNode::ChildUpdated(std::string_view child_name) {
		CommandNode& child = GetChild(child_name);

		for (auto it = alias_to_name_.begin(); it != alias_to_name_.end();) {
			if (it->second == child_name &&
				child.cmd_template_.aliases.find(it->first) == child.cmd_template_.aliases.end()) {

				it = alias_to_name_.erase(it);
			}
			else {
				++it;
			}
		}

		for (auto it = child.cmd_template_.aliases.begin(); it != child.cmd_template_.aliases.end();) {
			std::string_view alias = *it;
			if (!alias_to_name_.try_emplace(std::string{ alias }, std::string{ child_name }).second) {
				if (alias_to_name_.find(alias)->second != child_name) {
					it = child.cmd_template_.aliases.erase(it);
					alias_to_name_.emplace(std::string{ alias }, std::string{ child_name });
				}
				else {
					++it;
				}
			}
			else {
				++it;
			}
		}
	}

	bool CommandNode::AddNode(std::string_view name) {
		if (HasChild(name)) {
			return false;
		}

		auto result = sub_nodes_.emplace(std::string{ name }, CommandNode{ std::ref(*this), name });
		return result.second;
	}

	bool CommandNode::AddNode(std::string_view name, CommandTemplate cmd_template, CommandExecutor executor) {
		if (AddNode(name)) {
			CommandNode& node = GetChild(name);
			node.SetTemplate(std::move(cmd_template));
			node.SetExecutor(executor);
			return true;
		}

		return false;
	}

	std::string_view CommandNode::GetName() const noexcept {
		return name_;
	}

	std::string_view CommandNode::GetChildNameFromAlias(std::string_view alias) const {
		auto it = alias_to_name_.find(alias);
		return alias_to_name_.find(alias)->second;
	}

	CommandNode& CommandNode::GetChild(std::string_view name) {
		if (!HasChild(name)) {
			throw std::invalid_argument("no child found: " + std::string{ name });
		}
		return sub_nodes_.find(name)->second;
	}

	const CommandNode& CommandNode::GetChild(std::string_view name) const {
		if (!HasChild(name)) {
			throw std::invalid_argument("no child found: " + std::string{ name });
		}
		return sub_nodes_.find(name)->second;
	}

	bool CommandNode::HasChild(std::string_view name) const noexcept {
		return sub_nodes_.contains(name);
	}

	bool CommandNode::HasChildAlias(std::string_view name) const noexcept {
		return alias_to_name_.contains(name);
	}

	bool CommandNode::HasShortOption(char short_name) const noexcept {
		return short_to_full_opt_.contains(short_name);
	}

	const std::string& CommandNode::GetShortOptionName(char short_name) const {
		return short_to_full_opt_.at(short_name);
	}

	bool CommandNode::HasOption(std::string_view option_name) const noexcept {
		return cmd_template_.options.contains(option_name);
	}

	const CommandOption& CommandNode::GetOption(std::string_view option_name) const {
		if (!HasOption(option_name)) {
			throw std::invalid_argument("no option found: " + std::string{ option_name });
		}
		return cmd_template_.options.find(option_name)->second;
	}

	int CommandNode::GetRequiredOptionCount() const noexcept {
		return required_option_count_;
	}

	bool CommandNode::HasParent() const noexcept {
		return parent_ != nullptr;
	}

	const CommandNode& CommandNode::GetParent() const {
		if (parent_ == nullptr) {
			throw std::invalid_argument("node has no parent");
		}
		else {
			return *parent_;
		}
	}

	bool CommandNode::Remove(std::string_view name) {
		if (HasChildAlias(name)) {
			name = GetChildNameFromAlias(name);
		}

		if (HasChild(name)) {
			CommandNode& child = GetChild(name);
			for (const std::string& alias : child.cmd_template_.aliases) {
				alias_to_name_.erase(alias);
			}

			sub_nodes_.erase(sub_nodes_.find(name));
			return true;
		}
		return false;
	}

	void CommandNode::SetTemplate(CommandTemplate cmd_template) {
		this->cmd_template_ = std::move(cmd_template);
		if (parent_) {
			parent_->ChildUpdated(name_);
		}

		short_to_full_opt_.clear();
		for (auto& pair : cmd_template_.options) {
			short_to_full_opt_.try_emplace(pair.second.short_name, pair.first);
			required_option_count_ += pair.second.required;
		}
	}

	const CommandTemplate& CommandNode::GetTemplate() const noexcept {
		return cmd_template_;
	}

	const std::map<std::string, std::string, std::less<>>& CommandNode::GetChildAliasMapping() const noexcept {
		return alias_to_name_;
	}

	const std::map<char, std::string, std::less<>>& CommandNode::GetShortOptionMapping() const noexcept {
		return short_to_full_opt_;
	}

	void CommandNode::SetExecutor(CommandExecutor executor) noexcept {
		executor_ = executor;
	}

	CommandExecutor CommandNode::GetExecutor() const noexcept {
		return executor_;
	}

	void CommandNode::SetCommand(CommandTemplate cmd_template, CommandExecutor executor) {
		SetTemplate(std::move(cmd_template));
		SetExecutor(executor_);
	}

	CommandNode& CommandNode::operator>>(std::string_view cmd) {
		using namespace comad::utility;

		if (cmd.empty()) {
			throw std::invalid_argument("subcommand name cannot be empty");
		}
		if (HasWhitespace(cmd)) {
			throw std::invalid_argument("subcommand name cannot have whitespaces");
		}
		if (!HasChild(cmd)) {
			AddNode(cmd);
		}

		return GetChild(cmd);
	}

	CommandNode& CommandNode::operator|(std::string_view alias) {
		using namespace comad::utility;

		if constexpr (Verbose) comad_logger.MakeStream<LogLevel::DEBUG>() << "adding alias" << alias << " for node " <<  name_ << std::flush;
		if (alias.empty()) {
			throw std::invalid_argument("subcommand alias cannot be empty");
		}
		if (HasWhitespace(alias)) {
			throw std::invalid_argument("subcommand alias cannot have whitespaces");
		}

		cmd_template_.aliases.emplace(alias);

		if (parent_) {
			parent_->ChildUpdated(name_);
		}

		return *this;
	}

	CommandNode& CommandNode::operator=(CommandExecutor executor) {
		this->executor_ = executor;
		return *this;
	}
}
