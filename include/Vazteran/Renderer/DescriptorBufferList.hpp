#ifndef VAZTERAN_DESCRIPTORLIST_HPP
#define VAZTERAN_DESCRIPTORLIST_HPP

namespace vzt
{
	class DescriptorList
	{
	  public:
		DescriptorList(std::size_t baseElementNb, std::size_t elementSize);

		template <class HeldType>
		DescriptorList(std::size_t baseElementNb = 64);

	  private:
		std::size_t m_elementSize;
	};
} // namespace vzt

#endif // VAZTERAN_DESCRIPTORLIST_HPP
