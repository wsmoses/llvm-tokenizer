import fabric
from fabric import Connection
import boto3

ec2 = boto3.resource('ec2')
client = boto3.client('ec2')

"""
TODO's:
ensure security group has the NFS port open (TCP 2049, all ipv4)
create efs dir

How to run:
create_keypair
construct_instance (x 2, etc)
wait_for_instance
get_connections
install_efs
mount_efs
run_on_all / run on one
terminate_instances
"""

f1_ami_name = "ubuntu/images/hvm-ssd/ubuntu-xenial-16.04-amd64-server-20180126"

def get_f1_ami_id():
    """ Get the AWS F1 Developer AMI by looking up the image name -- should be region independent.
    """
    response = client.describe_images(Filters=[{'Name': 'name', 'Values': [f1_ami_name]}])
    assert len(response['Images']) == 1
    return response['Images'][0]['ImageId']

def construct_instance_market_options(instancemarket, spotinterruptionbehavior, spotmaxprice):
    """ construct the dictionary necessary to configure instance market selection
    (on-demand vs spot)
    See:
    https://boto3.readthedocs.io/en/latest/reference/services/ec2.html#EC2.ServiceResource.create_instances
    and
    https://docs.aws.amazon.com/AWSEC2/latest/APIReference/API_InstanceMarketOptionsRequest.html
    """
    instmarkoptions = dict()
    if instancemarket == "spot":
        instmarkoptions['MarketType'] = "spot"
        instmarkoptions['SpotOptions'] = dict()
        if spotmaxprice != "ondemand":
            # no value for MaxPrice means ondemand, so fill it in if spotmaxprice is not ondemand
            instmarkoptions['SpotOptions']['MaxPrice'] = spotmaxprice
        if spotinterruptionbehavior != "terminate":
            # if you have special interruption behavior, we also need to make the instance persistent
            instmarkoptions['SpotOptions']['InstanceInterruptionBehavior'] = spotinterruptionbehavior
            instmarkoptions['SpotOptions']['SpotInstanceType'] = "persistent"
        return instmarkoptions
    elif instancemarket == "ondemand":
        # empty dict = on demand
        return instmarkoptions
    else:
        assert False, "INVALID INSTANCE MARKET TYPE."

def get_default_vpc():
    return ec2.Vpc([vpc for vpc in client.describe_vpcs()['Vpcs'] if vpc['IsDefault']][0]['VpcId'])

def get_default_subnet(vpc=get_default_vpc()):
    return list(vpc.subnets.filter())[0]

def get_default_efs(subnet=get_default_subnet()):
    efsclient = boto3.client('efs')
    fs = efsclient.describe_file_systems()
    return [a for a in efsclient.describe_mount_targets(FileSystemId=fs['FileSystems'][0]['FileSystemId'])['MountTargets'] if a['SubnetId']==subnet.id][0]['FileSystemId']

def get_default_security_group():
    return ec2.SecurityGroup(client.describe_security_groups(Filters=[{"Name":"group-name","Values":["default"]}])['SecurityGroups'][0]['GroupId'])

def create_keypair(name):
    kpair = client.create_key_pair(KeyName='autophase-test-key', DryRun=False)
    
    import os

    with open(os.open(kpair['KeyName']+'.pem', os.O_CREAT | os.O_WRONLY, 0o400), 'w') as f:
        f.write(kpair['KeyMaterial'])
    return kpair
    
def launch_instances(KeyName, instancetype, count, instancemarket, spotinterruptionbehavior, spotmaxprice, subnet=get_default_subnet(), security_group=get_default_security_group()):
    """
    Launch an instance of type instancetype.
    This will randomly select an availability zone in the region to launch into.
    """
    assert security_group.vpc_id == vpc.id

    marketconfig = construct_instance_market_options(instancemarket, spotinterruptionbehavior, spotmaxprice)

    f1_image_id = get_f1_ami_id()

    instances = ec2.create_instances(ImageId=f1_image_id,
                                     EbsOptimized=True,
                                     BlockDeviceMappings=[
                                         {
                                             'DeviceName': '/dev/sdb',
                                             'NoDevice': '',
                                         },
                                     ],
                                     InstanceType=instancetype,
                                     MinCount=count,
                                     MaxCount=count,
                                     NetworkInterfaces=[
                                         {'SubnetId': subnet.id,
                                          'DeviceIndex': 0,
                                          'AssociatePublicIpAddress':True,
                                          'Groups':[security_group.id]}
                                     ],
                                     KeyName=KeyName,
                                     InstanceMarketOptions=marketconfig
                                     )

    return instances

""" Terminate instances when given a list of instance ids."""
def terminate_instances(instances):
    client.terminate_instances(InstanceIds=[i.id for i in instances], DryRun=False)
    
def wait_for_load(instances):
    bar = []
    for instance in instances:
        instance.wait_until_running()
        bar.append(instance.load())
    return instances

def get_connections(instances, kpair):
    connections = []
    for instance in instances:
        cx = Connection(instance.public_dns_name, user='ubuntu', connect_kwargs={"key_filename":[kpair['KeyName']+'.pem']})
        connections.append(cx)
    return connections

def run_on_all(connections, *args, **kwargs):
    from fabric.group import ThreadingGroup
    g = ThreadingGroup.from_connections(connections)
    res = g.run(*args, **kwargs)
    return res
    
def install_efs(connections):
    run_on_all(connections, """sudo apt-get update &&
sudo apt-get -y install binutils &&
rm -rf efs-utils &&
git clone https://github.com/aws/efs-utils &&
cd efs-utils &&
./build-deb.sh && 
sudo apt-get -y install ./build/amazon-efs-utils*deb""")
    
def mount_efs(connections, efs):
    run_on_all(connections, """sudo mkdir -p /mnt/efs &&
    sudo chown ubuntu:ubuntu /mnt/efs
    sudo mount -t efs """ + efs + """:/ /mnt/efs""")
