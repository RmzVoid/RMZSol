using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RMZGui
{
	public class Packet
	{
		public ulong flowId;
		public Source direction;
		public int length;
		public byte[] data;
	}

	public enum Source
	{
		NewConnection = 10,
		Disconnect = 20,
		FromServer = 0x1,
		FromClient = 0x10000
	}

	public class PacketDispatcher
	{
		public void DispatchPacket(DeviceIO device, Packet inboundPacket)
		{
			if(inboundPacket.direction != Source.NewConnection && inboundPacket.direction != Source.Disconnect)
				InjectPacket(device, inboundPacket);
		}

		public void InjectPacket(DeviceIO device, Packet outboundPacket)
		{
			device.Write(outboundPacket);
		}
	}


}
