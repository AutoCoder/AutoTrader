#ifdef MY_STRATEGY_EXPORT
#define STRATEGY_API __declspec(dllexport)
#else
#define STRATEGY_API __declspec(dllimport)
#endif