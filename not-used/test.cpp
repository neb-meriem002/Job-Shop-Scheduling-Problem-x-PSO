

struct A {
	[[nodiscard]] constexpr operator int() const noexcept {
		return 0;
	}
};

int main() {
	A a;
}