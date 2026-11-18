#pragma once
#include <string>
#include <utility>
#include <vector>

// One problem found in a machine file, and the value it is about.
struct Problem {
	std::string message;
	// The value's JSON pointer ("/instructions/ADD"); empty when not known.
	std::string where;
	// Only advice (an untidily spelled pattern): reported as a warning.
	bool warning = false;
};

// The problems the loader finds. Each is stamped with the value being
// read when it is added (ProblemsAt), so that a merged machine file can
// report it at the file that wrote that value (plan 0.2:E5.T2).
class Problems {
public:
	void push_back(std::string message) { items.push_back({.message = std::move(message), .where = where}); }
	void emplace_back(std::string message) { push_back(std::move(message)); }
	void warn(std::string message) {
		items.push_back({.message = std::move(message), .where = where, .warning = true});
	}
	void append(const Problems& other) { items.insert(items.end(), other.items.begin(), other.items.end()); }

	[[nodiscard]] bool empty() const { return items.empty(); }
	[[nodiscard]] auto begin() const { return items.begin(); }
	[[nodiscard]] auto end() const { return items.end(); }

private:
	friend class ProblemsAt;
	std::vector<Problem> items;
	std::string where;
};

// While it lives, problems added are about the value at `pointer`.
class ProblemsAt {
public:
	ProblemsAt(Problems& problems, std::string pointer) : problems(&problems), saved(std::move(problems.where)) {
		problems.where = std::move(pointer);
	}
	ProblemsAt(const ProblemsAt&) = delete;
	ProblemsAt(ProblemsAt&&) = delete;
	ProblemsAt& operator=(const ProblemsAt&) = delete;
	ProblemsAt& operator=(ProblemsAt&&) = delete;
	~ProblemsAt() { problems->where = std::move(saved); }

private:
	Problems* problems;
	std::string saved;
};
