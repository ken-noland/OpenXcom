#include <vector>
#include <functional>

namespace OpenXcom
{

template <typename FunctionSignature>
class MulticastDelegate : public std::vector<std::function<FunctionSignature>>
{
public:
	using Function = std::function<FunctionSignature>;

	/// Call all functions in the delegate.
	template <typename... Args>
	void call(Args&&... args)
	{
		for (Function& function : *this)
		{
			function(std::forward<Args>(args)...);
		}
	}

	/// Call all functions in reverse order.
	template <typename... Args>
	void rcall(Args&&... args)
	{
		for (auto it = this->rbegin(); it != this->rend(); ++it)
		{
			(*it)(std::forward<Args>(args)...);
		}
	}
};

} // namespace OpenXcom
