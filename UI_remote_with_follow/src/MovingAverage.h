template <typename T, typename Total, int N>
class Moving_Average
{
public:
	Moving_Average()
		: num_samples_(0), total_(0)
	{ }

	void operator()(T sample)
	{
		if (num_samples_ < N)
		{
			samples_[num_samples_++] = sample;
			total_ += sample;
		}
		else
		{
			T& oldest = samples_[num_samples_++ % N];
			total_ += sample - oldest;
			oldest = sample;
		}
	}

	operator double() const { return total_ / std::min(num_samples_, N); }

private:
	T samples_[N];
	int num_samples_;
	Total total_;
};