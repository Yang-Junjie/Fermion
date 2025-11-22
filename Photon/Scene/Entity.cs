using System;
using System.Runtime.CompilerServices;

namespace Fermion
{
	public class Entity
	{
		protected Entity() { ID = 0; } 

		internal Entity(ulong id)
		{
			ID = id;
		}

		public ulong ID;

	}

}
