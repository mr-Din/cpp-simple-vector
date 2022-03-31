#pragma once
#include "array_ptr.h"

#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <iostream>

#include <utility>
#include <iterator>

class ReserveProxyObj {
public:
	ReserveProxyObj(size_t capacity_to_reserve)
		: capacity_to_reserve_(capacity_to_reserve)
	{}
	size_t GetCapacity() {
		return capacity_to_reserve_;
	}
private:
	size_t capacity_to_reserve_;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
	return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
	using Iterator = Type*;
	using ConstIterator = const Type*;

	SimpleVector() noexcept = default;

	explicit SimpleVector(size_t size) 
		: size_(size), capacity_(size), items_(ArrayPtr<Type>(size))
	{
		std::fill(begin(), end(), Type());
	}

	SimpleVector(size_t size, const Type& value)
		: size_(size), capacity_(size), items_(ArrayPtr<Type>(size)) 
	{
		std::fill(begin(), end(), value);
	}

	SimpleVector(std::initializer_list<Type> init)
		: size_(init.size()), capacity_(init.size()), items_(ArrayPtr<Type>(init.size())) 
	{
		std::copy(init.begin(), init.end(), begin());
	}

	SimpleVector(ReserveProxyObj obj) 
		: size_(0), capacity_(obj.GetCapacity()), items_(ArrayPtr<Type>(obj.GetCapacity()))
	{

	}

	SimpleVector(const SimpleVector& other)
		: size_(other.size_), capacity_(other.capacity_)
		, items_(ArrayPtr<Type>(other.capacity_))
	{
		std::copy(other.begin(), other.end(), begin());
	}

	SimpleVector& operator=(const SimpleVector& rhs) {
		if (*this != rhs) {
			auto tmp(rhs);
			swap(tmp);
		}
		return *this;
	}

	SimpleVector(SimpleVector&& other) noexcept {
		swap(other);
	}

	SimpleVector& operator=(SimpleVector&&) = default;

	~SimpleVector() {

	}

	size_t GetSize() const noexcept {
		return size_;
	}

	size_t GetCapacity() const noexcept {
		return capacity_;
	}

	bool IsEmpty() const noexcept {
		return !size_;
	}
	
	Type& operator[](size_t index) noexcept {
		return items_[index];
	}

	const Type& operator[](size_t index) const noexcept {
		return items_[index];
	}

	Type& At(size_t index) {
		if (index >= size_) {
			throw std::out_of_range("Выход за границы массива!");
		}
		return items_[index];
	}

	const Type& At(size_t index) const {
		if (index >= size_) {
			throw std::out_of_range("Выход за границы массива!");
		}
		return items_[index];
	}

	void Clear() noexcept {
		size_ = 0;
	}

	void Resize(size_t new_size) {
		if (new_size < size_) {
			size_ = new_size;
		}
		else if (new_size < capacity_) {
			std::generate(begin() + size_, begin() + new_size, []() { return Type{}; });
			size_ = new_size;
		}
		else {
			size_t new_capacity = std::max(new_size, capacity_ * 2);
			ArrayPtr<Type> tmp(new_capacity);
			std::move(begin(), end(), tmp.Get());
			std::generate(tmp.Get() + size_, tmp.Get() + new_size, []() { return Type{}; });
			items_.swap(tmp);
			size_ = new_size;
			capacity_ = new_capacity;
		}
	}

	Iterator begin() noexcept {
		return items_.Get();
	}

	Iterator end() noexcept {
		return items_.Get() + size_;
	}

	ConstIterator begin() const noexcept {
		return items_.Get();
	}

	ConstIterator end() const noexcept {
		return items_.Get() + size_;
	}

	ConstIterator cbegin() const noexcept {
		return items_.Get();
	}

	ConstIterator cend() const noexcept {
		return items_.Get() + size_;
	}

	void PushBack(Type item) {
		if (size_ < capacity_) {
			items_[size_] = std::move(item);
			++size_;
		}
		else {
			Resize(size_ + 1);
			items_[size_ - 1] = std::move(item);
		}
	}

	Iterator Insert(ConstIterator pos, Type value) {
		Iterator it_return = const_cast<Iterator> (pos);
		if (size_ < capacity_) {
			std::move_backward(it_return, end(), end() + 1);
			*it_return = std::move(value);
			++size_;
			return it_return;
		}
		else {
			size_t new_size = size_ + 1;
			size_t new_capacity = std::max(new_size, capacity_ * 2);
			ArrayPtr<Type> tmp(new_capacity);
			std::move(begin(), it_return, tmp.Get());
			auto it_value = tmp.Get() + (it_return - begin());
			*it_value = std::move(value);
			std::move(it_return, end(), it_value + 1);
			
			items_.swap(tmp);
			size_ = new_size;
			capacity_ = new_capacity;
			return it_value;
		}
	}

	void PopBack() noexcept {
		assert(size_ > 0);
		--size_;
	}

	Iterator Erase(ConstIterator pos) {
		Iterator it_return = const_cast<Iterator> (pos);
		std::move(std::next(it_return), end(), it_return);
		--size_;

		return it_return;
	}

	void swap(SimpleVector& other) noexcept {
		items_.swap(other.items_);
		std::swap(size_, other.size_);
		std::swap(capacity_, other.capacity_);
	}
	
	void Reserve(size_t new_capacity) {
		if (new_capacity > capacity_) {
			ArrayPtr<Type> tmp(new_capacity);
			std::move(begin(), end(), tmp.Get());
			items_.swap(tmp);
			capacity_ = new_capacity;
		}
	}

private:
	size_t size_ = 0;
	size_t capacity_ = 0;
	ArrayPtr<Type> items_;
};


template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return (&lhs == &rhs) ||
		(lhs.GetSize() == rhs.GetSize() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(rhs < lhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
	return !(lhs < rhs);
}