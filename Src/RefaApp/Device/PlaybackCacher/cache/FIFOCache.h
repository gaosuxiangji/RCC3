#ifndef FIFOCACHE_H
#define FIFOCACHE_H

#include <list>
#include <unordered_map>
#include <mutex>

namespace agile_device
{
	template<typename key_t, typename value_t>
	class FIFOCache
	{
	public:
		typedef typename std::pair<key_t, value_t> key_value_pair_t;
		typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

		FIFOCache(size_t capacity) : capacity_(capacity) {}

		void resize(size_t capacity)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			capacity_ = capacity;
		}

		bool get(const key_t & key, value_t & value)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			auto it = cache_map_.find(key);
			if (it == cache_map_.end())
			{
				return false;
			}

			value = it->second->second;

			return true;
		}

		void put(const key_t & key, const value_t & value)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			cache_list_.push_back(key_value_pair_t(key, value));

			auto it = cache_map_.find(key);
			if (it != cache_map_.end())
			{
				cache_list_.erase(it->second);
				cache_map_.erase(it);
			}
			
			auto last = cache_list_.end();
			last--;
			cache_map_[key] = last;
			if (cache_map_.size() > capacity_)
			{
				auto first = cache_list_.begin();
				cache_map_.erase(first->first);
				cache_list_.pop_front();
			}
		}

		void clear()
		{
			std::lock_guard<std::mutex> lock(mutex_);

			cache_list_.clear();
			cache_map_.clear();
		}

	private:
		size_t capacity_;
		std::list<key_value_pair_t> cache_list_;
		std::unordered_map<key_t, list_iterator_t> cache_map_;
		std::mutex mutex_;
	};
}

#endif
