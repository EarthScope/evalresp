
import edu.iris.dmc.ws.util.RespUtil;
import edu.iris.dmc.service.ServiceUtil;
import edu.iris.dmc.service.StationService;
import edu.iris.dmc.fdsn.station.model.Network;
import java.util.List;
import java.io.FileInputStream;
import java.io.File;

import static java.lang.System.in;
import static java.lang.System.out;

/**
 * Very simple converter, based on email from Yazan Suleiman 
 * <yazan@iris.washington.edu>, used to generate correct output
 * for test comparison.
 */

public class Convert {

    public static void main(String[] args) throws Exception {
	ServiceUtil serviceManager;
	serviceManager = ServiceUtil.getInstance();
	StationService stationService = serviceManager.getStationService();
	List<Network> list = stationService.load(in);
	RespUtil.write(out, list);
    }

}
