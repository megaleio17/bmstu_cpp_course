#pragma once
#include <cstdint>
#include <exception>
#include <type_traits>

namespace bmstu
{
struct nullopt_t
{
	constexpr explicit nullopt_t(int) {}
};
inline constexpr nullopt_t nullopt{0};

class bad_optional_access : public std::exception
{
   public:
	using exception::exception;
	const char* what() const noexcept override { return "Bad optional access"; }
};

template <typename T>
class optional
{
   public:
	optional() = default;

	optional(const T& value)
	{
		new (data_) T(value);
		is_initialized_ = true;
	}

	optional(T&& value)
	{
		new (data_) T(std::move(value));
		is_initialized_ = true;
	}

	optional(const optional& other)
	{
		if (other.is_initialized_)
		{
			new (data_) T(*reinterpret_cast<const T*>(other.data_));
			is_initialized_ = true;
		}
	}

	optional(optional&& other) noexcept
	{
		if (other.is_initialized_)
		{
			new (data_) T(std::move(*reinterpret_cast<T*>(other.data_)));
			is_initialized_ = true;
			other.is_initialized_ = false;
		}
	}

	optional& operator=(const T& value)
	{
		if (is_initialized_)
		{
			*reinterpret_cast<T*>(data_) = value;
		}
		else
		{
			new (data_) T(value);
			is_initialized_ = true;
		}
		return *this;
	}

	optional& operator=(T&& value)
	{
		if (is_initialized_)
		{
			*reinterpret_cast<T*>(data_) = std::move(value);
		}
		else
		{
			new (data_) T(std::move(value));
			is_initialized_ = true;
		}
		return *this;
	}

	optional& operator=(const optional& other)
	{
		if (this != &other)
		{
			if (other.is_initialized_)
			{
				if (is_initialized_)
				{
					*reinterpret_cast<T*>(data_) =
						*reinterpret_cast<const T*>(other.data_);
				}
				else
				{
					new (data_) T(*reinterpret_cast<const T*>(other.data_));
					is_initialized_ = true;
				}
			}
			else
			{
				reset();
			}
		}
		return *this;
	}

	optional& operator=(optional&& other)
	{
		if (this != &other)
		{
			if (other.is_initialized_)
			{
				if (is_initialized_)
				{
					*reinterpret_cast<T*>(data_) =
						std::move(*reinterpret_cast<T*>(other.data_));
					other.is_initialized_ = false;
					reinterpret_cast<T*>(other.data_)->~T();
				}
				else
				{
					new (data_)
						T(std::move(*reinterpret_cast<T*>(other.data_)));
					is_initialized_ = true;
					other.is_initialized_ = false;
				}
			}
			else
			{
				reset();
			}
		}
		return *this;
	}

	T& operator*() &
	{
		if (!is_initialized_)
		{
			throw bad_optional_access{};
		}
		return *reinterpret_cast<T*>(data_);
	}

	const T& operator*() const&
	{
		if (!is_initialized_)
		{
			throw bad_optional_access{};
		}
		return *reinterpret_cast<const T*>(data_);
	}

	T* operator->()
	{
		if (!is_initialized_)
		{
			throw bad_optional_access{};
		}
		return reinterpret_cast<T*>(data_);
	}

	const T* operator->() const
	{
		if (!is_initialized_)
		{
			throw bad_optional_access{};
		}
		return reinterpret_cast<const T*>(data_);
	}

	T&& operator*() &&
	{
		if (!is_initialized_)
		{
			throw bad_optional_access{};
		}
		return std::move(*reinterpret_cast<T*>(data_));
	}

	T& value() &
	{
		if (!is_initialized_)
		{
			throw bad_optional_access{};
		}
		return *reinterpret_cast<T*>(data_);
	}

	const T& value() const&
	{
		if (!is_initialized_)
		{
			throw bad_optional_access{};
		}
		return *reinterpret_cast<const T*>(data_);
	}

	template <typename... Args>
	void emplace(Args&&... args)
	{
		reset();
		new (data_) T(std::forward<Args>(args)...);
		is_initialized_ = true;
	}

	void reset()
	{
		if (is_initialized_)
		{
			reinterpret_cast<T*>(data_)->~T();
			is_initialized_ = false;
		}
	}

	~optional() { reset(); }

	bool has_value() const { return is_initialized_; };

   private:
	alignas(T) uint8_t data_[sizeof(T)];
	bool is_initialized_ = false;
};
}  // namespace bmstu
